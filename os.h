/*
 * os.h
 *
 *  Created on: Oct 18, 2019
 *      Author: gta
 */

#include <msp430.h>
#include <types.h>
#include "intrinsics.h"
//#include <stdlib.h>

#ifndef OS_H_
#define OS_H_

#define NUMTHREADS 2
#define STACKSIZE 100

typedef int8_t thread_t; // index in tcbs array

void os_init(void);
void os_run(void) __attribute__ ((noreturn));
thread_t os_thread_create(void (*routine)(void));
void os_yield(void);

// test func
void os_thread_set(void (*routine1)(void), void (*routine2)(void));

//const word_t WDTCTL_DEFAULT = WDT_ADLY_1_9;


#endif /* OS_H_ */
