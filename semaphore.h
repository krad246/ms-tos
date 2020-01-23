/*
 * semaphore.h
 *
 *  Created on: Nov 16, 2019
 *      Author: krad2
 */

#ifndef SEMAPHORE_H_
#define SEMAPHORE_H_

#include <definitions.h>
#include <config.h>

/**
 * Initialize semaphore to value
 */

int sem_init(sem_t *sem, size_t value);

/**
 * Zero out the semaphore and clear its wait queue
 */

int sem_destroy(sem_t *sem);

/**
 * Blocking wait()
 */

int sem_wait(sem_t *sem);

/**
 * Blocking wait() with a time limit
 */

int sem_timedwait(sem_t *sem, size_t timeout);

/**
 * Non-blocking wait()
 */

int sem_trywait(sem_t *sem);

/**
 * Signal()
 */

int sem_post(sem_t *sem);

/**
 * Returns semaphore value
 */

int sem_getvalue(sem_t *sem);

#endif /* SEMAPHORE_H_ */
