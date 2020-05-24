/*
 * port_defs.h
 *
 *  Created on: May 23, 2020
 *      Author: krad2
 */

#ifndef PORTABLE_PORT_DEFS_H_
#define PORTABLE_PORT_DEFS_H_

#include "msp430.h"

/*-----------------------------------------------------------*/

#ifdef __LARGE_DATA_MODEL__
	#define PORT_STACK_TYPE uint32_t
#else
	#define PORT_STACK_TYPE uint16_t
#endif

typedef PORT_STACK_TYPE thread_stack_elem_t;

#if (CONFIG_USE_16_BIT_TICKS == 1)

#else

#endif

/*-----------------------------------------------------------*/

extern volatile uint16_t k_sched_us_nesting;
static void port_enter_critical(void) {
	if (__get_SR_register() & GIE) {
		__disable_interrupt();
		k_sched_us_nesting = 0;
	}
	k_sched_us_nesting++;
}

static void port_exit_critical(void) {
	k_sched_us_nesting--;
	if (k_sched_us_nesting == 0) {
		k_sched_us_nesting = 1;
		__enable_interrupt();
	}
}

/*-----------------------------------------------------------*/

extern void port_yield(void);

#endif /* PORTABLE_PORT_DEFS_H_ */
