/*
 * os.c
 *
 *  Created on: Jan 17, 2020
 *      Author: krad2
 */

#include <os.h>

/**
 * OS-allocated variables
 */

static size_t crit_sec_nest_counter = 0;
static os_task_t idle_thrd;
static struct pqueue_sleep sleep_queue;
static struct pqueue_timedwait t_waitqueue;
void *os_sp;
static size_t os_tick_cnt = 0;

/**
 * Memory pool for threads
 */

static struct pool_task_mem task_mem;

/**
 * Round robin scheduler variables
 */

static struct list_iterator it;
static struct list run_queue;
thrd_t *run_ptr;
static size_t task_active_cnt = 0;

static int os_idle(void *arg) {
	(void) arg;

	for (;;) {
		_low_power_mode_3();
	}
}

void os_panic(int error) {
	start_critical();
	os_idle(NULL);
}

void os_tick_init(void) {
	WDTCTL = WDT_ADLY_1_9;
	SFRIE1 |= WDTIE;
}

void os_tick_deinit(void) {
	WDTCTL = WDTPW | WDTHOLD;
}

/**
 * Allocates a process from the memory pool
 */

static os_task_t *talloc(void) {
	os_task_t *process;

	start_critical();
	process = pool_task_mem_alloc(&task_mem);
	end_critical();

	return process;
}

/**
 * Frees a process in the memory pool
 */

static void tfree(os_task_t *process) {
	start_critical();
	pool_task_mem_free(&task_mem, process);
	end_critical();
}

static int os_add_task_mmang(os_task_t *process) {
	if (process == NULL) return -1;

	int status;

	if (task_active_cnt == 0) list_iterator_init(&it, &run_queue); // no tasks - list vars have to be rebuilt

	status = list_append(&run_queue, &process->elem);
	if (status == -1) return status;

	task_active_cnt++;
	return status;
}

static int os_del_task_mmang(os_task_t *process) {
	if (process == NULL) return -1;

	int status;

	struct list_element *current;
	list_iterator_previous_circular(&it, &current);	// move backwards to avoid iterator invalidation

	status = list_remove(&run_queue, &process->elem);
	if (status == -1) return status;

	task_active_cnt--;
	return status;
}

static int os_add_task(os_task_t *process) {
	int status;

	start_critical();

	status = os_add_task_mmang(process);

	end_critical();

	return status;
}

static int os_del_task(os_task_t *process) {
	int status;

	start_critical();

	status = os_del_task_mmang(process);

	end_critical();

	return status;
}

static int os_mem_init(void) {
	start_critical();

	int status;
	status = pool_task_mem_init(&task_mem);	// Initialize memory pool and run queue for future allocations
	if (status < 0) return status;

	status = list_init(&run_queue);
	if (status < 0) return status;

	status = list_iterator_init(&it, &run_queue);

	end_critical();
	return status;
}

os_task_t *os_task_create(int (*routine)(void *), void *arg) {
	start_critical();

	// Allocate process (or at least try to)
	os_task_t *new_process = talloc();
	if (new_process == NULL) return NULL;	// OutOfMemory

	int status;

	// Initialize its other members for the run queue
	status = list_element_init(&new_process->elem);
	if (status < 0) return NULL;

	// Add to the run queue
	status = os_add_task(new_process);
	if (status < 0) return NULL;

	end_critical();

	return new_process;
}

static void os_task_yield(void) {
	SFRIFG1 |= WDTIFG;
}

const thrd_t *os_task_current(void) {
	thrd_t *curr;

	start_critical();
	curr = run_ptr;
	end_critical();

	return curr;
}

__attribute__((noreturn)) void os_task_kill(void) {
	start_critical();

	os_task_t *process = (os_task_t *) os_task_current();

	os_del_task(process);
	tfree(process);
	os_task_yield();

	end_critical();

	for (;;);
}

void os_task_pause(thrd_t *thrd) {
	start_critical();

	os_del_task((os_task_t *) thrd);
	os_task_yield();

	end_critical();
}

void os_task_unpause(thrd_t *thrd) {
	start_critical();

	thrd->state = ACTIVE;
	os_add_task((os_task_t *) thrd);

	end_critical();
}

void os_task_block(thrd_t *thrd) {
	start_critical();

	thrd->state = BLOCKING;
	os_task_pause(thrd);

	end_critical();
}

void os_update(void) {
	os_tick_cnt++;

	register volatile const size_t sq_cleanup_thresh = sleep_queue.count / 2;
	while (sleep_queue.count > sq_cleanup_thresh && --sleep_queue.storage[1].priority <= 0) {
		struct sleep_deadline deadline;
		pqueue_sleep_pop(&sleep_queue, &deadline);
		os_add_task_mmang((os_task_t *) deadline.data);
		deadline.data->state = ACTIVE;

		struct pqueue_sleep_node *next_deadline = &sleep_queue.storage[1];
		next_deadline->priority -= (deadline.ticks - 1);
		next_deadline->data.ticks += deadline.ticks;
	}

	register volatile const size_t t_wq_cleanup_thresh = t_waitqueue.count / 2;
	while (t_waitqueue.count > t_wq_cleanup_thresh && (--t_waitqueue.storage[1].priority <= 0 || t_waitqueue.storage[1].data.data->state == ACTIVE)) {
		struct sleep_deadline deadline;
		pqueue_timedwait_pop(&t_waitqueue, &deadline);
		os_add_task_mmang((os_task_t *) deadline.data);

		struct pqueue_timedwait_node *next_deadline = &t_waitqueue.storage[1];
		next_deadline->priority -= deadline.ticks;
		next_deadline->data.ticks += deadline.ticks;
	}
}

static void os_change_task(void) {
	#define CONTAINER_OF(ptr, type, field_name) ((type *)(((char *)ptr) - offsetof(type, field_name)))
	struct list_element *current;
	list_iterator_next_circular(&it, &current);
	run_ptr = (thrd_t *) CONTAINER_OF(current, os_task_t, elem);
}

static void os_next(void) {
	if (run_ptr->state == BLOCKING || run_ptr->state == SLEEPING || --run_ptr->working_prio == 0) {
		run_ptr->working_prio = run_ptr->fixed_prio;
		os_change_task();
	}
}

void os_schedule(void) {
	if (task_active_cnt > 0) os_next();
	else run_ptr = (thrd_t *) &idle_thrd;
}

void os_first_task(void) {
	if (task_active_cnt > 0) os_change_task();
	else run_ptr = (thrd_t *) &idle_thrd;
}

size_t os_time(void) {
	size_t ticks;

	start_critical();
	ticks = os_tick_cnt;
	end_critical();

	return ticks;
}

void os_task_sleep(thrd_t *caller, size_t ticks) {
	if (ticks == 0) {
		os_task_yield();
	} else {
		start_critical();

		struct sleep_deadline deadline = { caller, ticks };
		pqueue_sleep_push(&sleep_queue, ticks, &deadline);

		caller->state = SLEEPING;
		caller->working_prio = caller->fixed_prio;	// reset the caller's time slice

		end_critical();

		os_task_pause(caller);
	}
}

void os_task_timedwait(thrd_t *caller, size_t ticks) {
	if (ticks == 0) {
		os_task_yield();
	} else {
		start_critical();

		struct sleep_deadline deadline = { caller, ticks };
		pqueue_timedwait_push(&t_waitqueue, ticks, &deadline);

		caller->state = SLEEPING;
		caller->working_prio = caller->fixed_prio;	// reset the caller's time slice

		end_critical();

		os_task_pause(caller);
	}
}

void os_task_exit(int ret_code) {
	start_critical();

	thrd_t *this = (thrd_t *) os_task_current();

	// signal all of the threads on the wait queue
	while (queue_wait_count(&this->join_sem.wq) > 0) {
		// identify the thread to wake up
		volatile thrd_t *wakeup_thrd;
		queue_wait_back(&this->join_sem.wq, &wakeup_thrd);

		// write your return code to the private data section of the thread that we are going to wake up
		int *thrd_join_ret_val = (int *) &wakeup_thrd->mbox;
		*thrd_join_ret_val = ret_code;

		// wake it up
		extern int sem_post(sem_t *sem);
		sem_post(&this->join_sem);
	}

	end_critical();
}

void os_task_join(thrd_t *other, int * const ret_code) {
	const thrd_t *curr_thrd = os_task_current();
	extern int sem_wait(sem_t *s);
	sem_wait(&other->join_sem);
	*ret_code = curr_thrd->mbox;
}

int os_task_detach(thrd_t *thrd) {
	return 0;
}

void start_critical(void) {
	__disable_interrupt();
	crit_sec_nest_counter++;
}

void end_critical(void) {
	if (crit_sec_nest_counter > 0) {
		crit_sec_nest_counter--;

		if (crit_sec_nest_counter == 0) {
			__enable_interrupt();
		}
	}
}

void os_init(void) {
	int status;

	start_critical();

	status = os_mem_init();
	OS_ASSERT(status);

	extern int thrd_init(thrd_t *, int (*)(void *), void *, size_t);
	status = thrd_init((thrd_t *) &idle_thrd, os_idle, NULL, 1);
	OS_ASSERT(status);

	pqueue_sleep_init(&sleep_queue);
	pqueue_timedwait_init(&t_waitqueue);

	end_critical();
}

void os_launch(void) {
	extern void os_start(void);
	os_start();
}

__attribute__((noreturn)) void os_exit(void) {
	__disable_interrupt();
	os_tick_deinit();

	extern void os_cleanup(void);
	os_cleanup();

	for (;;);
}
