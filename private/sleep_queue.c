/*
 * sleep_queue.c
 *
 *  Created on: Jun 24, 2020
 *      Author: krad2
 */

#include "rtos.h"
#include "sleep_queue.h"

#define SWAP(x, y) do { typeof(x) SWAP = x; x = y; y = SWAP; } while (0)

void sleep_queue_init(sleep_queue_t *que) {
	rb_tree_lcached_init(&que->q);
}

static int sleepq_entry_cmp(const void *left, const void *right) {
	sleep_queue_entry_t *a = rb_entry_safe(left, sleep_queue_entry_t, node);
	sleep_queue_entry_t *b = rb_entry_safe(right, sleep_queue_entry_t, node);

	return (long) a->wake_time - (long) b->wake_time;
}

static void sleepq_entry_copy(const void *src, void *dst) {
	sleep_queue_entry_t *a = rb_entry_safe(src, sleep_queue_entry_t, node);
	sleep_queue_entry_t *b = rb_entry_safe(dst, sleep_queue_entry_t, node);
	b->thread = a->thread;
	b->wake_time = a->wake_time;
}

void sleep_queue_push(sleep_queue_t *que, thread_impl_t *thr, unsigned int wake_time) {
//	if (thr->sq_entry.thread) {
//	thr->sq_entry.thread->wake_time = wake_time;
//	thr->sq_entry.thread->thread = &thr->sq_entry;
//	rb_tree_lcached_insert(&que->q, &thr->sq_entry.thread->node, sleepq_entry_cmp);
//	} else {
//
//	}
//	thr->sq_entry.wake_time = wake_time;
//	thr->sq_entry.thread = &thr->sq_entry;
//	rb_tree_lcached_insert(&que->q, &thr->sq_entry.node, sleepq_entry_cmp);
//}
}
thread_impl_t *sleep_queue_peek(sleep_queue_t *que) {
	rb_node_t *ent_node = rb_min(&que->q);
	if (ent_node == 0) return 0;

	sleep_queue_entry_t *ent = rb_entry_safe(ent_node, sleep_queue_entry_t, node);
	return container_of(ent->thread, thread_impl_t, sq_entry);
}

thread_impl_t *sleep_queue_pop(sleep_queue_t *que) {
	rb_node_t *deleted;
	rb_tree_lcached_delete_at(&que->q, rb_min(&que->q), &deleted,sleepq_entry_cmp, sleepq_entry_copy);
	return container_of(container_of(deleted, sleep_queue_entry_t, node)->thread, thread_impl_t, sq_entry);
}

thread_impl_t *sleep_queue_remove_node(sleep_queue_t *que, thread_impl_t *thr) {
	rb_node_t *deleted;
//	rb_tree_lcached_delete_at(&que->q, &thr->sq_entry.node,&deleted, sleepq_entry_cmp, sleepq_entry_copy);
	return container_of(container_of(deleted, sleep_queue_entry_t, node)->thread, thread_impl_t, sq_entry);
}
