/*
 * mutex.h
 *
 *  Created on: Jan 22, 2020
 *      Author: krad2
 */

#ifndef MUTEX_H_
#define MUTEX_H_

#include <definitions.h>
#include <config.h>

/**
 * Initializes mutex
 */

int mtx_init(mtx_t *mtx);

/**
 * Mutex 'destructor'
 */

int mtx_destroy(mtx_t *mtx);

/**
 * Mutex lock() (similar to P() for a semaphore)
 */

int mtx_lock(mtx_t *mtx);

/**
 * Mutex lock() (similar to P() for a semaphore), but with a timeout
 */

int mtx_timedlock(mtx_t *mtx, size_t timeout);

/**
 * Non-blocking lock()
 */

int mtx_trylock(mtx_t *mtx);

/**
 * Mutex unlock() (similar to V() for a semaphore)
 */

int mtx_unlock(mtx_t *mtx);

#endif /* MUTEX_H_ */
