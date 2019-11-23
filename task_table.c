/*
 * task_table.c
 *
 *  Created on: Nov 16, 2019
 *      Author: krad2
 */

#include <task_table.h>

/**
 * Initializes the task table
 */

int task_table_init(void) {
	int status;
	status = pool_task_mem_init(&task_mem);	// Initialize memory pool and run queue for future allocations
	if (status < 0) return status;

	status = list_init(&task_table);
	if (status < 0) return status;

	status = list_iterator_init(&it, &task_table);
	return status;
}

/**
 * Allocates a process from the memory pool
 */

task_table_element_t *task_element_alloc(void) {
	task_table_element_t *process = pool_task_mem_alloc(&task_mem);
	return process;
}

/**
 * Frees a process in the memory pool
 */

void task_element_free(task_table_element_t *process) {
	pool_task_mem_free(&task_mem, process);
}

/**
 * Creates a task given a function address and arguments (can be NULL)
 */

int task_create(int (*routine)(void *), void *arg) {

	// Allocate process (or at least try to)
	task_table_element_t *new_process = task_element_alloc();
	if (new_process == NULL) return -1;	// OutOfMemory

	// If successful, initialize the process
	int status;
	status = thrd_init(&new_process->thread, routine, arg);
	if (status < 0) return status;

	// Initialize its other members for the run queue
	status = list_element_init(&new_process->elem);
	if (status < 0) return status;

	// Add to the run queue
	status = task_table_push(new_process);
	return status;
}

/**
 * Deletes a task given a pointer to it
 */

int task_kill(thrd_t *this) {

	// Find the underlying structure in the run queue
	task_table_element_t *process = (task_table_element_t *) this;
	if (process == NULL) return -1; // ProcessDoesNotExist

	// Unlink the task from the run queue
	int status;
	status = task_table_pop(process);
	if (status < 0) return status;

	// Deallocate the task
	task_element_free(process);
	return 0;
}

/**
 * Fetches current task
 */

const thrd_t *task_current(void) {
	return (const thrd_t *) run_ptr;
}

/**
 * Pushes task to table
 */

int task_table_push(task_table_element_t *process) {
	int status;
	status = list_append(&task_table, &process->elem);
	return status;
}

/**
 * Pops task from table
 */

int task_table_pop(task_table_element_t *process) {
	int status;
	status = list_remove(&task_table, &process->elem);
	return status;
}

/**
 * Fetches next task
 */

void task_next(void) {
	list_iterator_next_circular(&it, &current);								// Move through the circular list
	run_ptr = (thrd_t *) CONTAINER_OF(current, task_table_element_t, elem);	// Find the thread matching the node picked
}
