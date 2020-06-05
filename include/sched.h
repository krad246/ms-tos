/*
 * sched.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHED_H_
#define INCLUDE_SCHED_H_

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include "sched_flags.h"

typedef struct thread_t {
	void *sp;
} thread_t;


typedef enum {
	BORED
} thread_status_t;


extern volatile thread_t *sched_active_thread;
extern volatile int sched_context_switch_request;
extern sched_flags_t sched_status_flags;
extern uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];
extern volatile int __irq_is_in;

#include "sched_impl.h"

void sched_run(void);



#endif /* INCLUDE_SCHED_H_ */
