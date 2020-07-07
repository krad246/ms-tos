/*
 * panic.c
 *
 *  Created on: Jun 21, 2020
 *      Author: krad2
 */

#include <al.h>
#include "rtos.h"
#include "panic.h"

void __attribute__((naked, noreturn, noinline)) panic(panic_code_t crash_code, const char *message) {
	al_disable_interrupts();
	al_panic(crash_code, message);
	al_enable_interrupts();

	/* should never get here, but if it does, reboot */
	while (1) {
		al_panic(PANIC_HARD_REBOOT, "Critical system error, rebooting");
	}
}
