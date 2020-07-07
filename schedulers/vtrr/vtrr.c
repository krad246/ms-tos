/*
 * vtrr.c
 *
 *  Created on: Jun 21, 2020
 *      Author: krad2
 */

#include "vtrr.h"

/*-----------------------------------------------------------*/
#define swap(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)
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

/*-----------------------------------------------------------*/

#define VTRR_TIME_QUANTUM 			UINT_MAX
#define VTRR_TIMESTEP(shares)		VTRR_TIME_QUANTUM / ((shares))

/*-----------------------------------------------------------*/

/**
 * @name VTRR client member functions.
 * @{
 */

static void vtrr_client_init(vtrr_client_t *client, struct thread_impl *thread, unsigned int priority) {
	client->thread = thread;
	client->shares = priority;
	client->runs_left = priority;
	client->fin_time = 0;
	client->timestep = VTRR_TIMESTEP(client->shares);	/* precalculate the progress rate because division is slow */
}

static void vtrr_client_update(vtrr_client_t *client, unsigned int priority) {

	/* if priority is reduced and we've used more time than allowed, surrender the rest */
	if (priority < client->runs_left) client->runs_left = 0;

	/* recalculate the progress rate if the priority changed */
	if (client->shares != priority) client->timestep = VTRR_TIMESTEP(priority);
	client->shares = priority;
}

static bool vtrr_client_is_runnable(vtrr_client_t *client) {
	return client->runs_left > 0;
}

static void vtrr_client_run(vtrr_client_t *client) {

	/* the thread progresses proportional to its priority */
	if (client->runs_left > 0) {
		client->runs_left--;
		client->fin_time += client->timestep;
	}
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name rb_tree callbacks for vtrr_clients
 * @{
 */

/**
 * @brief Comparator callback between 2 clients.
 */
static int vtrr_client_cmp(const void *left, const void *right) {
    vtrr_client_t *a = vtrr_entry(left);
    vtrr_client_t *b = vtrr_entry(right);

    return a->shares - b->shares;
}

/**
 * @brief Copy node callback between 2 clients.
 */
static void vtrr_client_copy(const void *src, void *dst) {
    vtrr_client_t *a = vtrr_entry(src);
    vtrr_client_t *b = vtrr_entry(dst);

//    b->thread = a->thread;
//    b->shares = a->shares;
//    b->runs_left = a->runs_left;
//    b->fin_time = a->fin_time;
//    b->timestep = a->timestep;
    swap(b->thread, a->thread);
    swap(b->shares,a->shares);
    swap(b->runs_left,a->runs_left);
    swap(b->fin_time,a->fin_time);
    swap(b->timestep,a->timestep);
}

/**
 * @brief Foreach callback. Invoked at the end of every scheduling cycle.
 */
static void vtrr_client_reset(void *key) {
	vtrr_client_t *node = vtrr_entry(key);
	node->runs_left = node->shares;
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name Run queue functions.
 * @{
 */

/**
 * @brief Adds a thread to the run queue.
 */
static void vtrr_client_add_to_list(vtrr_mgr_t *mgr, struct thread_impl *thread,vtrr_client_t *client) {

	/* the arrival time of the task is the proportion of time in a cycle */
	client->fin_time = max(client->fin_time, mgr->group_time + client->timestep);

	/* scale the number of slices to the lateness of entry in the cycle */
	if (mgr->shares > 0) client->runs_left = (client->shares * mgr->runs_left) / mgr->shares;
	else client->runs_left = client->shares;

	/* Recalculate the manager state. */
	mgr->shares += client->shares;
	mgr->runs_left += client->runs_left;		/* lengthen the scheduling cycle */
	mgr->timestep = VTRR_TIMESTEP(mgr->shares);	/* recalculate the group timestep */

	rb_tree_rcached_insert(&mgr->rq, &client->node, vtrr_client_cmp);
}

/**
 * @brief Deletes a thread from the run queue.
 */
static void vtrr_client_remove_from_list(vtrr_mgr_t *mgr, vtrr_client_t *client) {
	mgr->shares -= client->shares;
	mgr->runs_left -= client->runs_left;		/* shorten the scheduling cycle */
	mgr->timestep = VTRR_TIMESTEP(mgr->shares);	/* recalculate the group timestep */

	rb_tree_rcached_delete_at(&mgr->rq, &client->node, NULL, vtrr_client_cmp, vtrr_client_copy);
}

/** @} */

/*-----------------------------------------------------------*/

/**
 * @name VTRR scheduling manager member functions.
 * @{
 */

#define vtrr_current_client(mptr) vtrr_entry((mptr)->curr_cli)
#define vtrr_next_client(mptr) vtrr_entry((mptr)->next_cli)

/**
 * @brief Sets up a scheduling instance.
 */
static void vtrr_mgr_init(vtrr_mgr_t *mgr) {
	rb_tree_rcached_init(&mgr->rq);

	mgr->curr_cli = NULL;
	mgr->next_cli = NULL;

	mgr->shares = 0;
	mgr->runs_left = 0;
	mgr->group_time = 0;
	mgr->timestep = 0;
}

/**
 * @brief Begins the scheduler, assuming at least 1 task is installed.
 */
static void vtrr_mgr_start(vtrr_mgr_t *mgr) {
	mgr->curr_cli = rb_max(&mgr->rq);
	mgr->next_cli = rb_max(&mgr->rq);
	mgr->timestep = VTRR_TIMESTEP(mgr->shares);	/* calculate the initial group timestep from the installed tasks */
}

/**
 * @brief Kills the scheduler.
 */
static void vtrr_mgr_end(vtrr_mgr_t *mgr) {
	mgr->curr_cli = NULL;
	mgr->next_cli = NULL;
	mgr->timestep = 0;

	rb_rcached_clean(&mgr->rq);					/* empties the run queue */
}

/**
 * @brief Iterates over all tasks and applies a callback to them.
 * @param[in] cb Function to apply per task.
 */
static void vtrr_mgr_task_foreach(vtrr_mgr_t *mgr, void (*cb)(void *)) {
	rb_inorder_foreach(&mgr->rq, cb);
}
//extern sched_impl_client_t done[10];
#include "sched.h"
extern struct vtrr_ clients[10];
/**
 * @brief Scheduling algorithm invoked by yield() and the timeslicer.
 */
static void vtrr_mgr_run(vtrr_mgr_t *mgr) {

	/* execution of the scheduled thread */
	mgr->curr_cli = mgr->next_cli;
	vtrr_client_t *curr_client = vtrr_current_client(mgr);
	#if (CONFIG_DEBUG_MODE == 1)
		if (curr_client == NULL) panic(PANIC_ASSERT_FAIL, "vtrr_run()::curr_max was NULL");
	#endif
	vtrr_client_run(curr_client);

	if (!vtrr_client_is_runnable(curr_client)) {
		rb_iterator_t deleted;
		rb_tree_rcached_delete_at(&mgr->rq, &curr_client->node, &deleted, vtrr_client_cmp, vtrr_client_copy);
		vtrr_entry(deleted)->runs_left = vtrr_entry(deleted)->shares;
		rb_tree_rcached_insert(&mgr->dq, deleted,  vtrr_client_cmp);
		if (rb_is_empty(&mgr->rq)) {
			swap(mgr->rq, mgr->dq);
			mgr->next_cli = rb_max(&mgr->rq);

		}

	}

	/* assign the thread previously planned for execution */

	mgr->group_time += mgr->timestep;
	if (mgr->runs_left > 0) mgr->runs_left--;

	/* if a cycle has completed */
	if (mgr->runs_left == 0) {

		/* go through all tasks and reset their run counters */
		vtrr_mgr_task_foreach(mgr, vtrr_client_reset);

		/* reset the manager's cycle counter */
		mgr->runs_left = mgr->shares;

		/* assign the next thread as the highest priority one */
		mgr->next_cli = rb_max(&mgr->rq);

		return;
	}

	/* assume that the next thread to be scheduled will be the next thread in sorted order */
	rb_node_t *next_node = (rb_node_t *) rb_prev(mgr->curr_cli);

	/* if it doesn't exist, just go back to the top */
	if (next_node == NULL) {
		mgr->next_cli = rb_max(&mgr->rq);
		return;
	}

	vtrr_client_t *next_client = vtrr_entry(next_node);

	/* if the next thread violates the 'even progress invariant' by not maintaining sorted order */
	if (next_client->runs_left > curr_client->runs_left) {
		mgr->next_cli = next_node;

	/* if the already-running client will allow enough time to squeeze this new thread in for a slice */
	} else if ( curr_client->fin_time < mgr->group_time +  (2 * curr_client->timestep)) {
		mgr->next_cli = next_node;

	/* default to maximum possible priority */
	} else {
		#if (CONFIG_DEBUG_MODE == 1)
		#endif
		mgr->next_cli = rb_max(&mgr->rq);;
	}
}

/**
 * @brief Surrenders timeslice by scheduling the next thread in order.
 */
static void vtrr_mgr_yield(vtrr_mgr_t *mgr) {
	vtrr_mgr_run(mgr);	/* rerunning vtrr_run() will move to the next step in the cycle */
}

/**
 * @brief Selects the highest priority runnable thread to be run, otherwise does nothing.
 */
static void vtrr_mgr_yield_higher(vtrr_mgr_t *mgr) {

	/* check if the highest priority runnable thread is worth it */
	if (vtrr_client_cmp(rb_max(&mgr->rq), mgr->curr_cli) > 0) {

		/* switch it and run it if that's true */
		mgr->next_cli = rb_max(&mgr->rq);
		vtrr_mgr_run(mgr);
	}
}

/** @} */

/*-----------------------------------------------------------*/

void vtrr_init(vtrr_mgr_t *sched) {
	vtrr_mgr_init(sched);
}

void vtrr_start(vtrr_mgr_t *sched) {
	vtrr_mgr_start(sched);
}

void vtrr_add(vtrr_mgr_t *sched, struct thread_impl *thread,vtrr_client_t *client, unsigned int priority) {
	vtrr_client_init(client, thread, priority);
	vtrr_client_add_to_list(sched, thread, client);
}

void vtrr_register(vtrr_mgr_t *sched, struct thread_impl *thread,vtrr_client_t *client) {
	vtrr_client_add_to_list(sched, thread,client);
}

void vtrr_deregister(vtrr_mgr_t *sched, vtrr_client_t *client) {
	vtrr_client_remove_from_list(sched, client);
}

void vtrr_reregister(vtrr_mgr_t *sched, struct thread_impl *thread,vtrr_client_t *client, unsigned int priority) {
	vtrr_deregister(sched, client);
	vtrr_client_update(client, priority);
	vtrr_register(sched, thread, client);
}

void vtrr_end(vtrr_mgr_t *sched) {
	vtrr_mgr_end(sched);
}

void vtrr_run(vtrr_mgr_t *sched) {
	vtrr_mgr_run(sched);
}

void vtrr_yield(vtrr_mgr_t *sched) {
	vtrr_mgr_yield(sched);
}

void vtrr_yield_higher(vtrr_mgr_t *sched) {
	vtrr_mgr_yield_higher(sched);
}
