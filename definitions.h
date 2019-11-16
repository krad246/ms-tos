/*
 * definitions.h
 *
 *  Created on: Nov 15, 2019
 *      Author: krad2
 */

#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <msp430.h>

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

/* Determine structure sizes */

#if defined(__LARGE_DATA_MODEL__)
	typedef uint32_t word_t;
#else
	typedef uint16_t word_t;
#endif

#if defined(__LARGE_CODE_MODEL__)
	typedef uint32_t pc_t;
#else
	typedef uint16_t pc_t;
#endif

#include <thread.h>

#endif /* DEFINITIONS_H_ */
