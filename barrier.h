/*
 * barrier.h
 *
 *  Created on: Nov 23, 2019
 *      Author: krad2
 */

#ifndef BARRIER_H_
#define BARRIER_H_

#include <definitions.h>
#include <semaphore.h>

typedef struct barrier_t {
	size_t count;
	size_t val;

	sem_t mutex;
	sem_t turnstile1;
	sem_t turnstile2;
} barrier_t;

int barrier_init(barrier_t *barrier, size_t value);
int barrier_wait(barrier_t *barrier);
int barrier_destroy(barrier_t *barrier);

#endif /* BARRIER_H_ */
