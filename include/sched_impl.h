/*
 * sched_impl.h
 *
 *  Created on: Jun 4, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_SCHEDULERS_SCHED_IMPL_H_
#define INCLUDE_SCHEDULERS_SCHED_IMPL_H_

#include "port_config.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SCHED_IMPL(type)													\
typedef type##_mgr_t sched_impl_mgr_t;										\
typedef type##_client_t sched_impl_client_t;								\
																			\
static void sched_impl_init(sched_impl_mgr_t *sched) {						\
	type##_init(sched);														\
}																			\
																			\
static void sched_impl_add(sched_impl_mgr_t *sched,							\
		sched_impl_client_t *client, unsigned int priority) {				\
	type##_add(sched, client, priority);									\
}																			\
																			\
static void sched_impl_register(sched_impl_mgr_t *sched,					\
		sched_impl_client_t *client) {										\
	type##_register(sched, client);											\
}																			\
																			\
static void sched_impl_deregister(sched_impl_mgr_t *sched,					\
		sched_impl_client_t *client) {										\
	type##_deregister(sched, client);										\
}																			\
																			\
static void sched_impl_reregister(sched_impl_mgr_t *sched,					\
		sched_impl_client_t *client, unsigned int priority) {				\
	type##_reregister(sched, client, priority);								\
}																			\
																			\
static void sched_impl_start(sched_impl_mgr_t *sched) {						\
	type##_start(sched);													\
}																			\
																			\
static void sched_impl_end(sched_impl_mgr_t *sched) {						\
	type##_end(sched);														\
}																			\
																			\
static void sched_impl_run(sched_impl_mgr_t *sched) {						\
	type##_run(sched);														\
}																			\
																			\
static void sched_impl_yield(sched_impl_mgr_t *sched) {						\
	type##_yield(sched);													\
}																			\
																			\
static void sched_impl_yield_higher(sched_impl_mgr_t *sched) {				\
	type##_yield_higher(sched);												\
}																			\

#define __sched_impl_active_client(type, mptr) type##_active_client(mptr)
#define sched_impl_active_client(mptr) 										\
				__sched_impl_active_client(vtrr, mptr)						\


// TODO: make the incldue part of the SCHED_IMPL macro so that the ifdefs are simpler
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


SCHED_IMPL(vtrr);

#ifdef __cplusplus
}
#endif


#endif /* INCLUDE_SCHEDULERS_SCHED_IMPL_H_ */
