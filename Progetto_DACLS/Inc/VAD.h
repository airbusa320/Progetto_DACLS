/*
 * VAD.h
 *
 *      Author: Stefano,Riccardo
 */

#ifndef VAD_H_
#define VAD_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include "math.h"
#define ARM_MATH_CM4
#include "arm_math.h"
#include "arm_const_structs.h"

typedef enum
{
	NON_ATTIVO=0,
	ATTIVO,

} marker;

uint8_t VAD_AE(float32_t* frame,uint16_t frame_length/*, uint8_t i */);

#endif /* VAD_H_ */


