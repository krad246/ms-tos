/*
 * bitarithm.h
 *
 *  Created on: May 23, 2020
 *      Author: krad2
 */

#ifndef CORE_BITARITHM_H_
#define CORE_BITARITHM_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SETBIT(val, bit)    val |= (bit)
#define CLRBIT(val, bit)    val &= (~(bit))




#ifdef __cplusplus
}
#endif

#endif /* CORE_BITARITHM_H_ */
