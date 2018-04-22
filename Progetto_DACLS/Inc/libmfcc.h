/*
 * libmfcc.h
 *
 *      Author: Stefano,Riccardo
 */
#include "stm32f4xx_hal.h"

#include "math.h"
#define ARM_MATH_CM4
#include "arm_math.h"

#pragma once

//#define PI 3.14159265358979323846264338327

// Returns the specified (mth) MFCC
float32_t GetCoefficient(float32_t* spectralData, uint16_t samplingRate, uint8_t NumFilters, uint16_t binSize, uint8_t m);

// Compute the normalization factor (For internal computation only - not to be called directly)
float32_t NormalizationFactor(uint8_t NumFilters, uint8_t m);

// Compute the filter parameter for the specified frequency and filter bands (For internal computation only - not the be called directly)
float32_t GetFilterParameter(uint16_t samplingRate, uint16_t binSize, unsigned int frequencyBand, unsigned int filterBand);
// Compute the band-dependent magnitude factor for the given filter band (For internal computation only - not the be called directly)
float32_t GetMagnitudeFactor(unsigned int filterBand);

// Compute the center frequency (fc) of the specified filter band (l) (For internal computation only - not the be called directly)
float32_t GetCenterFrequency(unsigned int filterBand);
