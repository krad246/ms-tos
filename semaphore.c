/*
 * semaphore.c
 *
 *  Created on: Nov 16, 2019
 *      Author: krad2
 */

#include <semaphore.h>

/**
 * Initializes semaphore to value
 */

int sem_init(sem_t *sem, size_t value) {
	if (!sem) return -1;	// InvalidArgument
	if (value > NUM_THREADS) return -1;	// InvalidArgument

	_start_critical();

	sem->val = value;						// Starting value of semaphore
	queue_wait_init(&sem->wq);				// Initialize the wait queue and clear it out
	memset(&sem->wq, 0, sizeof(sem->wq));

	_end_critical();

	return 0;
}

/**
 * Semaphore 'destructor'
 */

int sem_destroy(sem_t *sem) {
	if (!sem) return -1;	// InvalidArgument

	_start_critical();

	sem->val = 0;			// Semaphore will kill any thread that tries to wait on it now

	volatile thrd_t *dummy;
	while (queue_wait_count(&sem->wq) > 0) queue_wait_pop(&sem->wq, &dummy);	// Clears the queue

	_end_critical();

	return 0;
}

/**
 * Blocking semaphore P()
 */

int sem_wait(sem_t *sem) {
	if (!sem) return -1;	// InvalidArgument

	_start_critical();

	--sem->val;	// Down() on the semaphore
	if (sem->val < 0) {	// If no resources remaining, block
		volatile const thrd_t *calling_thread = os_get_current_thread();	// Fetch the calling thread
		queue_wait_push(&sem->wq, &calling_thread);	// Add its handle to the wait queue

		_os_task_freeze((thrd_t *) calling_thread);	// Unlink it from the task table (but don't delete it)

		os_yield();	// Transfer control to the scheduler
	}

	_end_critical();

	return 0;
}

/**
 * Non-blocking semaphore P()
 */

int sem_trywait(sem_t *sem) {
	if (!sem) return -1;	// InvalidArgument

	_start_critical();

	int status = -1;
	if (sem->val > 0) {		// If the semaphore is free, then acquire and return normally
		--sem->val;
		status = 0;
	}						// Else return an error

	_end_critical();

	return status;
}

/**
 * Semaphore V()
 */

int sem_post(sem_t *sem) {
	if (!sem) return -1;

	_start_critical();

	++sem->val; 								// If val exceeds NUM_THREADS, then panic?
	if (sem->val <= 0) {						// If the semaphore has any threads waiting on it, then wake up one thread
		volatile thrd_t *wakeup;
		queue_wait_pop(&sem->wq, &wakeup);		// Pop and return the wakeup thread from the queue

		_os_task_unfreeze((thrd_t *) wakeup); 	// Relink it into the task table
	}

	_end_critical();

	return 0;
}

int sem_getvalue(sem_t *sem) {
	_start_critical();
	int val = sem->val;
	_end_critical();

	return val;
}
