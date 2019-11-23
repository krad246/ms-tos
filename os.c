/*
 * os.c
 *
 *  Created on: Nov 16, 2019
 *      Author: krad2
 */

#include <os.h>

thrd_t _os_idle_thrd;
size_t task_cnt = 0;
int _os_idle_hook(void *arg) {
	_low_power_mode_3();
	while (1);
}

void _start_critical(void) {
	__disable_interrupt();
}

void _end_critical(void) {
	__enable_interrupt();
}

// Set up tick timer
void os_tick_init(void) {
	UCSCTL5 = DIVA_1;
	WDTCTL = WDT_ADLY_1_9;
	SFRIE1 |= WDTIE;
}

void os_tick_reset(void) {
	WDTCTL = WDT_ADLY_1_9;
	SFRIFG1 &= ~WDTIE;
}

// Initialize OS
void os_init(void) {
	int status;

	_start_critical();

	status = task_table_init();			// Initialize the run queue
	if (status < 0) os_panic(status);	// If failed, panic

	status = thrd_init(&_os_idle_thrd, _os_idle_hook, NULL);
	if (status < 0) os_panic(status);

	_end_critical();
}

// Run the OS
void os_launch(void) {
	_start_critical();

	os_schedule();						// Schedule the first task
	_os_start();						// Branch to the task

	_end_critical();

	os_panic(0);
}

// Create a thread
int os_thread_create(int (*routine)(void *), void *arg) {
	int status;

	_start_critical();
	status = thrd_create(routine, arg);
	if (status == 0) task_cnt++;
	_end_critical();

	return status;
}

void os_sleep(size_t ticks) {
	_start_critical();
	volatile const thrd_t *calling_thread = os_get_current_thread();
//	calling_thread->sleep_cnt = ticks;
//	queue_wait_push(&sem->wq, &calling_thread); -- replace with sleeping priority queue

	_os_task_freeze((thrd_t *) calling_thread);

	os_yield();
	_end_critical();
}

// Pick the next task
// If no tasks available to run, then set run_ptr to the idle thread
size_t os_tick_cnt = 0;
void os_schedule(void) {
	if (task_cnt == 0) os_idle();
	else task_next();

	++run_ptr->exec_cnt;
	os_tick_cnt++;
}

void os_idle(void) {
	run_ptr = &_os_idle_thrd;
}


// Returns a handle to the current thread
const thrd_t *os_get_current_thread(void) {
	return task_current();
}

// Transfer control to the scheduler
void os_yield(void) {
	SFRIFG1 |= WDTIFG;
}

// Blocks a process
void _os_task_freeze(thrd_t *process) {
	volatile task_table_element_t *entry = (task_table_element_t *) process;	// Fetch the calling thread entry
	struct list_element links;
	links.prev = entry->elem.prev;
	links.next = entry->elem.next;

	task_table_pop((task_table_element_t *) process); task_cnt--;				// Pop() removes the links but we need them

	entry->elem.prev = links.prev;												// Retain the references to the list members so that we can transfer control successfully (will never be called again)
	entry->elem.next = links.next;
}

// Unblocks a process
void _os_task_unfreeze(thrd_t *process) {
	volatile task_table_element_t *entry = (task_table_element_t *) process;	// Fetch the wakeup thread entry
	struct list_element links;
	links.prev = entry->elem.prev;
	links.next = entry->elem.next;

	links.prev->next = (struct list_element *) &entry->elem;					// Relink it into the list
	links.next->prev = (struct list_element *) &entry->elem;

	task_cnt++;
}

// Kernel panic
void os_panic(int error) {
	_start_critical();

	// Find the faulting application here (fetch return address from stack)

	while (1);
}
