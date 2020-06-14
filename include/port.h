/*
 * port.h
 *
 *  Created on: May 23, 2020
 *      Author: krad2
 */

#ifndef INCLUDE_PORT_H_
#define INCLUDE_PORT_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
	#include "compat/gcc/port_defs.h"
#else
	#ifdef __TI_COMPILER_VERSION__
		#include "compat/ccs/port_defs.h"
	#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* INCLUDE_PORT_H_ */
