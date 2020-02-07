/*
 * semaphore.c
 *
 *  Created on: Nov 16, 2019
 *      Author: krad2
 */

#include <semaphore.h>
#include <os.h>

/**
 * Initializes semaphore to value
 */

int sem_init(sem_t *sem, size_t value) {
	if (!sem) return -1;	// InvalidArgument
	if (value > NUM_THREADS) return -1;	// InvalidArgument

	start_critical();

	sem->val = value;						// Starting value of semaphore
	queue_wait_init(&sem->wq);				// Initialize the wait queue and clear it out
	memset(&sem->wq, 0, sizeof(sem->wq));

	end_critical();

	return 0;
}

/**
 * Semaphore 'destructor'
 */

int sem_destroy(sem_t *sem) {
	if (!sem) return -1;	// InvalidArgument

	start_critical();
	sem->val = 0;			// Semaphore will kill any thread that tries to wait on it now
	queue_wait_clear(&sem->wq);
	end_critical();

	return 0;
}

/**
 * Blocking semaphore P()
 */

int sem_wait(sem_t *sem) {
	if (!sem) return -1;	// InvalidArgument

	start_critical();

	if (--sem->val < 0) {	// Down() on the semaphore; if no resources remaining, block
		extern const thrd_t *os_task_current(void);
		volatile thrd_t *calling_thread = (volatile thrd_t *) os_task_current();	// Fetch the calling thread

		queue_wait_push(&sem->wq, (const volatile thrd_t **) &calling_thread);	// Add its handle to the wait queue

		extern void os_task_block(thrd_t *thr);
		os_task_block((thrd_t *) calling_thread);	// Unlink it from the task table (but don't delete it)
	}

	end_critical();

	return 0;
}

int sem_timedwait(sem_t *sem, size_t timeout) {
	if (!sem) return -1;	// InvalidArgument

	int status = 0;
	volatile uint8_t locked = 1;

	start_critical();

	extern const thrd_t *os_task_current(void);
	volatile thrd_t *calling_thread;
	if (--sem->val < 0) {	// Down() on the semaphore; if no resources remaining, sleep up to the timeout
		locked = 0;			// wakeup reason needs to be determined from this

		calling_thread = (volatile thrd_t *) os_task_current();	// Fetch the calling thread

		extern void os_task_timedwait(thrd_t *caller, size_t ticks);
		queue_wait_push(&sem->wq, (volatile const thrd_t **) &calling_thread);	// Add its handle to the wait queue

		os_task_timedwait((thrd_t *) calling_thread, timeout);	// Unlink it from the task table (but don't delete it)
	}

	end_critical();

	// wakeup region starts here - check if you were signalled externally (e.g. not on the wait queue)
	// assumption is that the queue is valid between its indices; if you were signalled, then you would no longer lie within the queue indices

	start_critical();

	if (!locked) {																// if we failed to acquire the semaphore before
		if (calling_thread->state == SLEEPING) {								// if we're still marked as sleeping, then we did not get reawakened manually
			calling_thread->state = ACTIVE;
			sem_post(sem);
			status = -1;
		}
	}

	end_critical();

	return status;
}

/**
 * Non-blocking semaphore P()
 */

int sem_trywait(sem_t *sem) {
	if (!sem) return -1;	// InvalidArgument

	start_critical();

	int status = -1;
	if (sem->val > 0) {		// If the semaphore is free, then acquire and return normally
		--sem->val;
		status = 0;
	}						// Else return an error

	end_critical();

	return status;
}

/**
 * Semaphore V()
 */

int sem_post(sem_t *sem) {
	if (!sem) return -1;

	start_critical();

	if (++sem->val <= 0) {						// If the semaphore has any threads waiting on it, then wake up one thread
		volatile thrd_t *wakeup;

		queue_wait_pop(&sem->wq, &wakeup);		// Pop and return the wakeup thread from the queue

		extern void os_task_unpause(thrd_t *thr);
		os_task_unpause((thrd_t *) wakeup); 	// Relink it into the task table
	}

	end_critical();

	return 0;
}

int sem_getvalue(sem_t *sem) {
	start_critical();
	int val = sem->val;
	end_critical();

	return val;
}
