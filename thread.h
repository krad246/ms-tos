/*
 * thread.h
 *
 *  Created on: Nov 15, 2019
 *      Author: krad2
 */

#ifndef THREAD_H_
#define THREAD_H_

#include <definitions.h>
#include <config.h>

/**
 * Initializes an allocated thread
 */

/**
 * Wrapper function to initialize a thread
 */

int thrd_create(int (*routine)(void *), void *arg, size_t priority);

/**
 * Transfers control to the scheduler
 */ 

void thrd_yield(void);

/**
 * Puts a thread to sleep for the specified amount of time, or at least that much.
 */
void thrd_sleep(size_t ticks);

/**
 * Returns a handle to the current thread running in the system
 */

const thrd_t *thrd_current(void);

/**
 * Comparator for threads through their handles
 */

int thrd_equal(thrd_t *lhs, thrd_t *rhs);

/**
 * Waits on a thread to return
 */

void thrd_join(thrd_t *other, int * const ret_code);

/**
 * Detaches a thread e.g. makes it unjoinable.
 */

int thrd_detach(thrd_t *thr);

#endif /* THREAD_H_ */
