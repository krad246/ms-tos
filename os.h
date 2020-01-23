/*
 * os.h
 *
 *  Created on: Jan 17, 2020
 *      Author: krad2
 */

#ifndef OS_H_
#define OS_H_

#include <definitions.h>
#include <config.h>

#define OS_ASSERT(status) if (status < 0) os_panic(status)

void start_critical(void);
void end_critical(void);

void os_init(void);
void os_launch(void);
__attribute__((noreturn)) void os_exit(void);

#endif /* OS_H_ */
