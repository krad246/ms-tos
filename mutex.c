/*
 * mutex.c
 *
 *  Created on: Jan 22, 2020
 *      Author: krad2
 */

#include <mutex.h>
#include <os.h>

/**
 * Initializes mutex
 */

int mtx_init(mtx_t *mtx) {
	if (!mtx) return -1;	// InvalidArgument

	start_critical();

	mtx->holder = NULL;						// Starting value of semaphore
	queue_wait_init(&mtx->wq);				// Initialize the wait queue and clear it out
	memset(&mtx->wq, 0, sizeof(mtx->wq));

	end_critical();

	return 0;
}

/**
 * Mutex 'destructor'
 */

int mtx_destroy(mtx_t *mtx) {
	if (!mtx) return -1;	// InvalidArgument

	start_critical();
	mtx->holder = NULL;			// Semaphore will kill any thread that tries to wait on it now
	queue_wait_clear(&mtx->wq);
	end_critical();

	return 0;
}

/**
 * Mutex lock() (similar to P() for a semaphore)
 */

int mtx_lock(mtx_t *mtx) {
	if (!mtx) return -1;

	start_critical();

	extern const thrd_t *os_task_current(void);
	if (mtx->holder == NULL) {
		mtx->holder = (thrd_t *) os_task_current();
	} else {
		extern void os_task_pause(thrd_t *thr);
		volatile const thrd_t *calling_thread = os_task_current();	// Fetch the calling thread
		queue_wait_push(&mtx->wq, &calling_thread);	// Add its handle to the wait queue
		os_task_pause((thrd_t *) calling_thread);	// Unlink it from the task table (but don't delete it)
	}

	end_critical();

	return 0;
}

int mtx_timedlock(mtx_t *mtx, size_t timeout) {
	if (!mtx) return -1;

	int status = 0;
	volatile uint8_t locked = 1;
	start_critical();

	extern const thrd_t *os_task_current(void);
	volatile const thrd_t *calling_thread = os_task_current();	// Fetch the calling thread
	size_t push_idx;	// we will save the location on the wait queue
	if (mtx->holder == NULL) {
		mtx->holder = (thrd_t *) calling_thread;
	} else {
		locked = 0;

		extern void os_task_sleep(thrd_t *caller, size_t ticks);
		queue_wait_push(&mtx->wq, &calling_thread);	// Add its handle to the wait queue
		push_idx = mtx->wq.write;

		os_task_sleep((thrd_t *) calling_thread, timeout);	// Unlink it from the task table (but don't delete it)
	}

	end_critical();

	start_critical();

	if (!locked) {
		if (mtx->holder != (thrd_t *) calling_thread) {		// if we were woken up and passed the mutex, then no problem - else the acquisition failed
			mtx->wq.storage[push_idx] = NULL;				// mark our calling thread entry as free and drop it from consideration
			mtx_unlock(mtx);
			status = -1;
		}
	}

	end_critical();

	return status;
}

int mtx_trylock(mtx_t *mtx) {
	if (!mtx) return -1;	// InvalidArgument

		start_critical();

		int status = -1;

		extern const thrd_t *os_task_current(void);
		volatile const thrd_t *calling_thread = os_task_current();	// Fetch the calling thread
		if (mtx->holder != NULL) {		// If the semaphore is free, then acquire and return normally
			mtx->holder = (thrd_t *) calling_thread;
			status = 0;
		}						// Else return an error

		end_critical();

		return status;
}

/**
 * Mutex unlock() (similar to V() for a semaphore)
 */

int mtx_unlock(mtx_t *mtx) {
	if (!mtx) return -1;

	int status = 0;
	start_critical();

	extern const thrd_t *os_task_current(void);
	if (mtx->holder != os_task_current()) {
		status = -1;
	} else {
		if (queue_wait_count(&mtx->wq) > 0) {
			volatile thrd_t *wakeup;

			queue_wait_pop(&mtx->wq, &wakeup);		// Pop and return the wakeup thread from the queue
			mtx->holder = (thrd_t *) wakeup;

			extern void os_task_unpause(thrd_t *thr);
			os_task_unpause((thrd_t *) wakeup); 	// Relink it into the task table
		} else {
			mtx->holder = NULL;
		}
	}

	end_critical();

	return status;
}
