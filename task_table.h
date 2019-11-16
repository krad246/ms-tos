/*
 * run_queue.h
 *
 *  Created on: Nov 16, 2019
 *      Author: krad2
 */

#ifndef TASK_TABLE_H_
#define TASK_TABLE_H_

#include <definitions.h>
#include <config.h>

#include <list.h>
#include <pool.h>
#include <thread.h>

#define CONTAINER_OF(ptr, type, field_name) ((type *)(((char *)ptr) - offsetof(type, field_name)))

/**
 * Run queue linked list structure and corresponding element definition
 */

struct list task_table;
typedef struct task_table_element {
	thrd_t thread;
	struct list_element elem;
} task_table_element_t;

/**
 * Create a memory pool for threads
 */

POOL(task_mem, task_table_element_t, NUM_THREADS);
struct pool_task_mem task_mem;

/**
 * Initializes the thread table
 */

int task_table_init(void);

/**
 * Allocates / frees threads
 */

task_table_element_t *task_element_alloc(void);
void task_element_free(task_table_element_t *process);

/**
 * Create / destroy threads
 */

int task_create(int (*routine)(void *), void *arg);
int task_kill(thrd_t *this);

struct list_iterator it;
struct list_element *current;
task_table_element_t *run_ptr;

/**
 * Fetches the next task from the run queue
 */

void task_next(void);

#endif /* TASK_TABLE_H_ */
