/*
 * os.h
 *
 *  Created on: Nov 16, 2019
 *      Author: krad2
 */

#ifndef OS_H_
#define OS_H_

#include <definitions.h>
#include <task_table.h>

// Enters / exits critical section (disables interrupt)
void _start_critical(void);
void _end_critical(void);

// Sets up preemptive tick
void os_tick_init(void);

// Sets up os memory, state variables, etc.
void os_init(void);

// Underlying branch to first task
extern void _os_start(void);

// Wrapper; picks first runnable and calls _os_start()
void os_launch(void);

// Creates a thread given a runnable and an argument; wrapper
int os_thread_create(int (*routine)(void *), void *arg);

// Picks a new thread to run
void os_schedule(void);

// Voluntary transfer of control to the scheduler
void os_yield(void);

// Error handler for the kernel
void os_panic(int error);

#endif /* OS_H_ */
