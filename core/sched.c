/*
 * sched.c
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#include "sched.h"

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