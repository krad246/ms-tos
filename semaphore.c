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
		extern void os_task_pause(thrd_t *thr);
		volatile thrd_t *calling_thread = (volatile thrd_t *) os_task_current();	// Fetch the calling thread
		queue_wait_push(&sem->wq, (const volatile thrd_t **) &calling_thread);	// Add its handle to the wait queue

		os_task_pause((thrd_t *) calling_thread);	// Unlink it from the task table (but don't delete it)
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
	size_t push_idx;	// we will save the location on the wait queue
	if (--sem->val < 0) {	// Down() on the semaphore; if no resources remaining, sleep up to the timeout
		locked = 0;			// wakeup reason needs to be determined from this

		calling_thread = (volatile thrd_t *) os_task_current();	// Fetch the calling thread

		extern void os_task_sleep(thrd_t *caller, size_t ticks);
		queue_wait_push(&sem->wq, (volatile const thrd_t **) &calling_thread);	// Add its handle to the wait queue
		push_idx = sem->wq.write;	// save that

		os_task_sleep((thrd_t *) calling_thread, timeout);	// Unlink it from the task table (but don't delete it)
	}

	end_critical();

	// wakeup region starts here - check if you were signalled externally (e.g. not on the wait queue)
	// assumption is that the queue is valid between its indices; if you were signalled, then you would no longer lie within the queue indices

	start_critical();

	if (!locked) {																// if we failed to acquire the semaphore before
		const size_t left = sem->wq.read;										// check the intervals if we woke up and are still inside the wait queue
		const size_t right = sem->wq.write;

		if (left < right) {														// case read < write (normal queue behavior)
			if (left < push_idx && push_idx <= right) {							// if where we were put on the queue is contained within the range, then we were not signalled
				sem->wq.storage[push_idx] = NULL;								// then mark it as a free entry in the queue (scheduler will just jump over it later)
				sem_post(sem);													// reset the state of the semaphore and return an error
				status = -1;
			}
		} else if (right < left) {												// other case for modular arithmetic (write wraps around)
			// basically check the complement (e.g. before it was read < idx && idx <= write, roles switch here)
			// in that case, the interval spanned by (write, read] is the set of elements NOT in the queue -> if idx is not in this range, it is in the queue
			if (!(right < push_idx && push_idx <= left)) {
				sem->wq.storage[push_idx] = NULL;
				sem_post(sem);
				status = -1;
			}
		} else {																// worst case, the queue is full, in which case we have to linear search to check
			if (queue_wait_count(&sem->wq) > 0) {
				size_t i;
				for (i = 0; i < sizeof(sem->wq.storage) / sizeof(*sem->wq.storage); ++i) {
					if (sem->wq.storage[i] == calling_thread) {
						sem->wq.storage[push_idx] = NULL;
						sem_post(sem);
						status = -1;
						break;
					}
				}
			}
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
