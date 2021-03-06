/*
 * my_lib.h
 *
 *  Created on: 27 apr 2018
 *      Author: Riccardo
 */

#ifndef MY_LIB_H_
#define MY_LIB_H_

#include "stm32f4xx_hal.h"
#include "main.h"
#include "i2s.h"
#include "usart.h"
#include "pdm2pcm_glo.h"
#define ARM_MATH_CM4
#include "arm_math.h"

#define MAXINT16 32768

enum
{
	SX=0,
	DX=1,

} ultimo;

void StartAcquisition();
void int2float(int16_t * psrc, float32_t * pDst, uint16_t size);
void Process();
void PDM2PCM(uint8_t *PDMBuf, uint8_t *PCMBuf);

#endif /* MY_LIB_H_ */
