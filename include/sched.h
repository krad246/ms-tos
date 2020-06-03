/*
 * sched.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHED_H_
#define INCLUDE_SCHED_H_

#include <stdint.h>

#include "sched_flags.h"

typedef struct thread_t {
	void *sp;
} thread_t;



extern volatile thread_t *sched_active_thread;
extern volatile int sched_context_switch_request;
extern sched_flags_t sched_status_flags;
extern uint8_t sched_isr_stack[CONFIG_ISR_STACK_SIZE];
extern volatile int __irq_is_in;

void sched_run(void);

extern bool port_interrupts_enabled(void);
extern void port_disable_interrupts(void);
extern void port_enable_interrupts(void);

static void sched_enter_critical(void) {
	if (port_interrupts_enabled()) {
		port_disable_interrupts();
		sched_flags_cs_set(&sched_status_flags, 0);	// replace with discrete variable modifications if event groups are unselected
	}

	sched_flags_cs_increment(&sched_status_flags);
}

static void sched_exit_critical(void) {
	sched_flags_cs_decrement(&sched_status_flags);

	if (sched_flags_cs_get(&sched_status_flags) == 0) {
		sched_flags_cs_set(&sched_status_flags, 1);
		port_enable_interrupts();
	}
}

#endif /* INCLUDE_SCHED_H_ */
