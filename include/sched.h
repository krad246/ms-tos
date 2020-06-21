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

extern volatile int sched_context_switch_request;
extern sched_flags_t sched_status_flags;
extern uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];
extern volatile int __irq_is_in;

#include "sched_impl.h"


typedef struct thread {
	void *sp;
	vtrr_client_t cli;
} thread_t;

extern volatile thread_t *sched_active_thread;

typedef enum {
	BORED
} thread_status_t;

void sched_run(void);
void sched_yield_higher(void);


#endif /* INCLUDE_SCHED_H_ */
