/*
 * sched_flags.h
 *
 *  Created on: Jun 2, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHED_FLAGS_H_
#define INCLUDE_SCHED_FLAGS_H_

#include <stdint.h>

#include "panic.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCHED_FLAG_LOCKED (1u << 15)
#define SCHED_FLAG_IN_IRQ (1u << 14)
#define SCHED_FLAG_CONTEXT_SWITCH_REQUESTED (1u << 13)

#define SCHED_STATUS_MASK (SCHED_FLAG_LOCKED | SCHED_FLAG_IN_IRQ | SCHED_FLAG_CONTEXT_SWITCH_REQUESTED)
#define SCHED_CRITICAL_NESTING_MASK (~SCHED_STATUS_MASK)

#define SCHED_CS(x) ((x) & SCHED_CRITICAL_NESTING_MASK)
#define SCHED_STATUS(x) ((x) & SCHED_STATUS_MASK)

#define SCHED_FLAGS_INITIAL_VALUE (1)

typedef uint16_t sched_flags_t;

static void sched_flags_cs_set(sched_flags_t *ptr_flags, sched_flags_t mask) {
	*ptr_flags = SCHED_STATUS(*ptr_flags) | SCHED_CS(mask);
}

static sched_flags_t sched_flags_cs_get(sched_flags_t *ptr_flags) {
	return SCHED_CS(*ptr_flags);
}

static void sched_flags_cs_increment(sched_flags_t *ptr_flags) {
	#if (CONFIG_CHECK_CRITICAL_SECTION_VALIDITY == 1)
		*ptr_flags = SCHED_STATUS(*ptr_flags) | SCHED_CS(*ptr_flags + 1);
	#else
		*ptr_flags -= 1;
	#endif
}

static void sched_flags_cs_decrement(sched_flags_t *ptr_flags) {
	#if (CONFIG_CHECK_CRITICAL_SECTION_VALIDITY == 1)
		*ptr_flags = SCHED_STATUS(*ptr_flags) | SCHED_CS(*ptr_flags - 1);
	#else
		*ptr_flags -= 1;
	#endif
}

static void sched_set_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask) {
	*ptr_flags |= SCHED_STATUS(mask);
}

static void sched_clear_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask) {
	*ptr_flags &= ~SCHED_STATUS(mask);
}

static void sched_assign_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask) {
	sched_clear_flags(ptr_flags, mask);
	sched_set_flags(ptr_flags, mask);
}

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_SCHED_FLAGS_H_ */
