/*
 * thread.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_THREAD_H_
#define INCLUDE_THREAD_H_

#include "sched.h"
#include "sleep_queue.h"
#include "thread_impl.h"

typedef struct thread {
	thread_impl_t base;
	irq_lock_t cs_lock;
} thread_t;

#endif /* INCLUDE_THREAD_H_ */
