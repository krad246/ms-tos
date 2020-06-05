/*
 * sched_impl.h
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHEDULERS_SCHED_IMPL_H_
#define INCLUDE_SCHEDULERS_SCHED_IMPL_H_

#include "port_config.h"

#if (CONFIG_SCHED_RR == 1)
	#include "schedulers/rr/rr.h"
#elif (CONFIG_SCHED_VTRR == 1)
	#include "schedulers/vtrr/vtrr.h"
#elif (CONFIG_SCHED_LOTTERY == 1)
	#include "schedulers/lottery/lottery.h"
#elif (CONFIG_SCHED_MULTIQ == 1)
	#include "schedulers/lottery/multiq.h"
#else
	#error "No scheduling algorithm configured by CONFIG_SCHED_*"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_SCHEDULERS_SCHED_IMPL_H_ */
