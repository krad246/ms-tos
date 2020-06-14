/*
 * vtrr.h
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHEDULERS_VTRR_H_
#define INCLUDE_SCHEDULERS_VTRR_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <limits.h>

#include "rbtree.h"

#ifndef max
#define max(a,b) \
	({ __typeof__ (a) _a = (a); \
	   __typeof__ (b) _b = (b); \
	 _a > _b ? _a : _b; })
#endif

#ifndef min
#define min(a,b) \
  ({ __typeof__ (a) _a = (a); \
      __typeof__ (b) _b = (b); \
    _a < _b ? _a : _b; })
#endif

#define SCHED_VTRR_QUANTUM UINT_MAX

typedef struct sched_vtrr_client {
	unsigned int shares;
	unsigned int runs_left;
	unsigned int fin_time;

	rb_node rq_entry;
} vtrr_client_t;

static int vtrr_client_cmp(const void *left, const void *right) {
    vtrr_client_t *a = rb_entry_safe(left, vtrr_client_t, rq_entry);
    vtrr_client_t *b = rb_entry_safe(right, vtrr_client_t, rq_entry);

    return a->shares - b->shares;
}

static void vtrr_client_copy(const void *src, void *dst) {
    vtrr_client_t *a = rb_entry_safe(src, vtrr_client_t, rq_entry);
    vtrr_client_t *b = rb_entry_safe(dst, vtrr_client_t, rq_entry);

    b->shares = a->shares;
    b->runs_left = a->runs_left;
    b->fin_time = a->fin_time;
}

static void vtrr_client_begin_new_cycle(void *key) {
	vtrr_client_t *node = rb_entry_safe(key, vtrr_client_t, rq_entry);
	node->runs_left = node->shares;
}

typedef struct sched_vtrr_mgr {
	unsigned int shares;
	unsigned int runs_left;
	unsigned int group_time;

	rb_tree_rcached rq;
	rb_node *rq_iterator;
} vtrr_mgr_t;

static void vtrr_client_init(vtrr_client_t *client, unsigned int priority) {
	client->shares = priority;
	client->runs_left = priority;
	client->fin_time = 0;
	rb_node_init(&client->rq_entry);
}

static void vtrr_client_add_to_list(vtrr_mgr_t *mgr, vtrr_client_t *client) {
	client->fin_time = max(client->fin_time, mgr->group_time + (SCHED_VTRR_QUANTUM / client->shares));

	if (mgr->shares > 0) client->runs_left = (client->shares * mgr->runs_left) / mgr->shares;
	else client->runs_left = client->shares;

	rb_insert_rcached(&mgr->rq, &client->rq_entry, vtrr_client_cmp);

	mgr->shares += client->shares;
	mgr->runs_left += client->runs_left;
}

static void vtrr_client_remove_from_list(vtrr_mgr_t *mgr, vtrr_client_t *client) {
	rb_delete_rcached(&mgr->rq, &client->rq_entry, vtrr_client_cmp, vtrr_client_copy);

	mgr->shares -= client->shares;
	mgr->runs_left -= client->runs_left;
}

static void vtrr_client_update_list_pos(vtrr_mgr_t *mgr, vtrr_client_t *client) {
	vtrr_client_remove_from_list(mgr, client);
	vtrr_client_add_to_list(mgr, client);
}

static bool vtrr_client_is_runnable(vtrr_client_t *client) {
	return client->runs_left > 0;
}

static void vtrr_client_run(vtrr_client_t *client) {
	if (client->runs_left > 0) {
		client->runs_left--;
		client->fin_time += SCHED_VTRR_QUANTUM / client->shares;
	}
}

static void vtrr_mgr_init(vtrr_mgr_t *mgr) {
	rb_tree_init(&mgr->rq.tree);
	mgr->rq.rightmost = NULL;
	mgr->rq_iterator = NULL;

	mgr->shares = 0;
	mgr->runs_left = 0;
	mgr->group_time = 0;
}

static void vtrr_mgr_start(vtrr_mgr_t *mgr) {
	mgr->rq_iterator = rb_last_cached(&mgr->rq);

}

static void vtrr_mgr_end(vtrr_mgr_t *mgr) {
	mgr->rq_iterator = rb_last_cached(&mgr->rq);
	// delete all nodes
}

// call this after grabbing sched_active_thread because this will point to the NEXT thread to be run.
static int vtrr_mgr_run(vtrr_mgr_t *mgr) {
	vtrr_client_t *curr_client = rb_entry_safe(mgr->rq_iterator, vtrr_client_t, rq_entry);

	if (vtrr_client_is_runnable(curr_client)) {
		vtrr_client_run(curr_client);

		mgr->group_time += SCHED_VTRR_QUANTUM / mgr->shares;
		mgr->runs_left--;

		if (mgr->runs_left == 0) {
			rb_inorder_foreach(&mgr->rq, vtrr_client_begin_new_cycle);
			mgr->rq_iterator = rb_last_cached(&mgr->rq);
			mgr->runs_left = mgr->shares;
			return 1;
		}
	}

	rb_node *next_node = rb_prev(mgr->rq_iterator);
	vtrr_client_t *next_client = rb_entry_safe(next_node, vtrr_client_t, rq_entry);

	if (next_client->runs_left > curr_client->runs_left) {
		mgr->rq_iterator = next_node;
		return 1;
	} else {
		// vtrr inequality is old_vft < group_time + q / curr.shares which implies an addition of q/curr.shares on both sides since we already ran curr.
		if (curr_client->fin_time < mgr->group_time + 2 * (SCHED_VTRR_QUANTUM / curr_client->shares)) {
			mgr->rq_iterator = next_node;
		} else {
			mgr->rq_iterator = rb_last_cached(&mgr->rq);
		}

		return 1;
	}

}

// the NEXT logical thread after the currently running thread will be run, then rq_iterator points to the new NEXT (i.e. 2 steps ahead).
// grab sched_active_thread before calling this.
static void vtrr_mgr_yield(vtrr_mgr_t *mgr) {
	vtrr_mgr_run(mgr);	// move in the order of the logical next process
}

// the new NEXT is pointed to by rq_iterator.
// grab sched_active_thread after calling this.
// then call vtrr_mgr_run().
static void vtrr_mgr_yield_higher(vtrr_mgr_t *mgr) {
	// move to the highest priority runnable node, starting from the highest priority
	rb_node *node;
	vtrr_client_t *client;

	do {
		node = rb_last_cached(&mgr->rq);
		client = rb_entry_safe(node, vtrr_client_t, rq_entry);
	} while (!vtrr_client_is_runnable(client)); // could be optimized using a __clz implementation and an array of pointers to nodes.

	mgr->rq_iterator = node; // when we call run, we will resume our loop from the highest node.
}

// sched_change_thread = set_sched_active_thread(); vtrr_mgr_run();

// the currently running process (i.e. 1 before rq_iterator) will be removed
// sched_active_thread will point to the currently running process.
// if sched_set_status sleeps sched_active_thread, then we will switch to the new thread and remove sched_active_thread from the list.
// default behavior ^ so we can just remove the client from the list (client arg == sched_active_thread)

// the time service will migrate the client to the timer queue.
static void vtrr_mgr_sleep(vtrr_mgr_t *mgr, vtrr_mgr_t *client) {
	vtrr_mgr_remove_from_list(mgr, client);	// simply remove the entry from the list
}

// similarly, waking up an arbitrary thread means adding it back into the list. called by the time service.
static void vtrr_mgr_wakeup(vtrr_mgr_t *mgr, vtrr_client_t *client) {
	vtrr_mgr_add_to_list(mgr, client);
}

#endif /* INCLUDE_SCHEDULERS_VTRR_H_ */
