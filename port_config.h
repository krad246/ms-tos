/*
 * port_config.h
 *
 *  Created on: May 23, 2020
 *      Author: krad2
 */

#ifndef PORT_CONFIG_H_
#define PORT_CONFIG_H_

//#define CONFIG_USE_PORT_OPTIMIZED_TYPES

#define CONFIG_NUM_COOP_PRIORITIES
#define CONFIG_NUM_PREEMPT_PRIORITIES
#define CONFIG_PREEMPT_THRESHOLD

#define CONFIG_USE_16_BIT_TICKS 1
#define CONFIG_USE_TICKLESS_IDLE
#define CONFIG_TICK_VECTOR WDT_VECTOR

#define CONFIG_USE_KERNEL_STACK
#define CONFIG_ISR_STACK_SIZE 256

#define CONFIG_HEAP_SIZE

#define CONFIG_USE_IDLE_HOOK                     					0
#define CONFIG_USE_TICK_HOOK                     					0
#define CONFIG_CHECK_CRITICAL_SECTION_VALIDITY						1
#define CONFIG_CHECK_FOR_STACK_OVERFLOW          					0
#define CONFIG_USE_MALLOC_FAILED_HOOK           		 			0


#endif /* PORT_CONFIG_H_ */
