/*
 * os.c
 *
 *  Created on: Nov 16, 2019
 *      Author: krad2
 */

#include <os.h>

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

// Initialize OS
void os_init(void) {
	int status;

	_start_critical();

	status = task_table_init();			// Initialize the run queue
	if (status < 0) os_panic(status);	// If failed, panic

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
	_end_critical();

	return status;
}

// Pick the next task
void os_schedule(void) {
	task_next();
}

// Transfer control to the scheduler
void os_yield(void) {
	SFRIFG1 |= WDTIFG;
}

// Kernel panic
void os_panic(int error) {
	_start_critical();

	// Find the faulting application here (fetch return address from stack)

	while (1);
}
