/*
 * sched_flags.c
 *
 *  Created on: Jun 3, 2020
 *      Author: krad2
 */

#include "sched_flags.h"

void sched_flags_cs_set(sched_flags_t *ptr_flags, sched_flags_t mask) {
	*ptr_flags = SCHED_STATUS(*ptr_flags) | SCHED_CS(mask);
}

sched_flags_t sched_flags_cs_get(sched_flags_t *ptr_flags) {
	return SCHED_CS(*ptr_flags);
}

void sched_flags_cs_increment(sched_flags_t *ptr_flags) {
	#if (CONFIG_CHECK_CRITICAL_SECTION_VALIDITY == 1)
		*ptr_flags = SCHED_STATUS(*ptr_flags) | SCHED_CS(*ptr_flags + 1);
	#else
		*ptr_flags -= 1;
	#endif
}

void sched_flags_cs_decrement(sched_flags_t *ptr_flags) {
	#if (CONFIG_CHECK_CRITICAL_SECTION_VALIDITY == 1)
		*ptr_flags = SCHED_STATUS(*ptr_flags) | SCHED_CS(*ptr_flags - 1);
	#else
		*ptr_flags -= 1;
	#endif
}

void sched_set_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask) {
	*ptr_flags |= SCHED_STATUS(mask);
}

void sched_clear_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask) {
	*ptr_flags &= ~SCHED_STATUS(mask);
}

void sched_assign_status_flags(sched_flags_t *ptr_flags, sched_flags_t mask) {
	sched_clear_status_flags(ptr_flags, mask);
	sched_set_status_flags(ptr_flags, mask);
}
