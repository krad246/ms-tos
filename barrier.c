/*
 * barrier.c
 *
 *  Created on: Nov 23, 2019
 *      Author: krad2
 */

#include <barrier.h>

int barrier_init(barrier_t *barrier, size_t value) {
	if (!barrier) return -1;
	if (value > NUM_THREADS) return -1;

	_start_critical();

	barrier->count = 0;
	barrier->val = value;
	sem_init(&barrier->mutex, 1);
	sem_init(&barrier->turnstile1, 0);
	sem_init(&barrier->turnstile2, 0);

	_end_critical();

	return 0;
}

void phase1_barrier(barrier_t *barrier) {
	sem_wait(&barrier->mutex);

	if (++barrier->count == barrier->val) {
		size_t i;
		for (i = 0; i < barrier->val; ++i) sem_post(&barrier->turnstile1);
	}

	sem_post(&barrier->mutex);
	sem_wait(&barrier->turnstile1);
}

void phase2_barrier(barrier_t *barrier) {
	sem_wait(&barrier->mutex);

	if (--barrier->count == 0) {
		size_t i;
		for (i = 0; i < barrier->val; ++i) sem_post(&barrier->turnstile2);
	}

	sem_post(&barrier->mutex);
	sem_wait(&barrier->turnstile2);
}

int barrier_wait(barrier_t *barrier) {
	if (!barrier) return -1;

	phase1_barrier(barrier);
	phase2_barrier(barrier);

	return 0;
}

int barrier_destroy(barrier_t *barrier) {
	if (!barrier) return -1;

	_start_critical();

	barrier->count = 0;
	barrier->val = 0;
	sem_destroy(&barrier->mutex);
	sem_destroy(&barrier->turnstile1);
	sem_destroy(&barrier->turnstile2);

	_end_critical();

	return 0;
}
