/*
 * thread.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_THREAD_H_
#define INCLUDE_THREAD_H_

typedef int (*thread_fn_t)(void *);

typedef struct thread {
	void *sp;
	irq_lock_t cs_lock;

	sched_impl_client_t cli;
} thread_t;

#endif /* INCLUDE_THREAD_H_ */
