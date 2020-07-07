/*
 * sleep_queue.h
 *
 *  Created on: Jun 24, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SLEEP_QUEUE_H_
#define INCLUDE_SLEEP_QUEUE_H_

#include "rbtree_lcached.h"

typedef struct thread_impl thread_impl_t;

typedef struct sleep_queue_entry {
	struct sleep_queue_entry *thread;
	unsigned int wake_time;
	rb_node_t node;
} sleep_queue_entry_t;

typedef struct sleep_queue {
	rb_tree_lcached_t q;
} sleep_queue_t;

void sleep_queue_init(sleep_queue_t *que);

void sleep_queue_push(sleep_queue_t *que, thread_impl_t *thr, unsigned int wake_time);

thread_impl_t *sleep_queue_peek(sleep_queue_t *que);

thread_impl_t *sleep_queue_pop(sleep_queue_t *que);

thread_impl_t *sleep_queue_remove_node(sleep_queue_t *que, thread_impl_t *thr);

#endif /* INCLUDE_SLEEP_QUEUE_H_ */
