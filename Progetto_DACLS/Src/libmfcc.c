/*
 * libmfcc.c
 *
 *      Author: Stefano,Riccardo
 */



#include "libmfcc.h"

/* 
 * Computes the specified (mth) MFCC
 *
 * spectralData - array of doubles containing the results of FFT computation. This data is already assumed to be purely real
 * samplingRate - the rate that the original time-series data was sampled at (i.e 44100)
 * NumFilters - the number of filters to use in the computation. Recommended value = 26
 * binSize - the size of the spectralData array, usually a power of 2
 * m - The mth MFCC coefficient to compute
 *
 */
float32_t GetCoefficient(float32_t* spectralData, uint16_t samplingRate, uint8_t NumFilters, uint16_t binSize, uint8_t m)
{
	float32_t result = 0.0f;
	float32_t outerSum = 0.0f;
	float32_t innerSum = 0.0f;
	uint16_t k, l;
	float32_t Source_abs = 0.0f;
	float32_t Dest_abs = 0.0f;
	// 0 <= m < L
	if(m >= NumFilters)
	{
		// This represents an error condition - the specified coefficient is greater than or equal to the number of filters. The behavior in this case is undefined.
		return 0.0f;
	}

	result = NormalizationFactor(NumFilters, m);


	for(l = 1; l <= NumFilters; l++)
	{
		// Compute inner sum
		innerSum = 0.0f;
		for(k = 0; k < binSize - 1; k++)
		{
			Source_abs = (spectralData[k] * GetFilterParameter(samplingRate, binSize, k, l));
			arm_abs_f32(&Source_abs,&Dest_abs,1);
			innerSum += Dest_abs;		//fabs(spectralData[k] * GetFilterParameter(samplingRate, binSize, k, l));

		}

		if(innerSum > 0.0f)
		{
			innerSum = logf(innerSum); // The log of 0 is undefined, so don't use it
		}


		innerSum = innerSum * arm_cos_f32(((m * PI) / NumFilters) * (l - 0.5f));



		outerSum += innerSum;
	}

	result *= outerSum;

	return result;
}

/* 
 * Computes the Normalization Factor (Equation 6)
 * Used for internal computation only - not to be called directly
 */
float32_t NormalizationFactor(uint8_t NumFilters, uint8_t m)
{
	float32_t normalizationFactor = 0.0f;

	if(m == 0)
	{
		//normalizationFactor = sqrt(1.0f / NumFilters);

		arm_sqrt_f32(1.0f / NumFilters,&normalizationFactor);
	}
	else 
	{
		//normalizationFactor = sqrt(2.0f / NumFilters);

		arm_sqrt_f32(2.0f / NumFilters,&normalizationFactor);
	}

	return normalizationFactor;
}

/* 
 * Compute the filter parameter for the specified frequency and filter bands (Eq. 2)
 * Used for internal computation only - not the be called directly
 */
float32_t GetFilterParameter(uint16_t samplingRate, uint16_t binSize, unsigned int frequencyBand, unsigned int filterBand)
{
	float32_t filterParameter = 0.0f;

	float32_t boundary = (frequencyBand * samplingRate) / binSize;		// k * Fs / N
	float32_t prevCenterFrequency = GetCenterFrequency(filterBand - 1);		// fc(l - 1) etc.
	float32_t thisCenterFrequency = GetCenterFrequency(filterBand);
	float32_t nextCenterFrequency = GetCenterFrequency(filterBand + 1);

	if(boundary >= 0 && boundary < prevCenterFrequency)
	{
		filterParameter = 0.0f;
	}
	else if(boundary >= prevCenterFrequency && boundary < thisCenterFrequency)
	{
		filterParameter = (boundary - prevCenterFrequency) / (thisCenterFrequency - prevCenterFrequency);
		filterParameter *= GetMagnitudeFactor(filterBand);
	}
	else if(boundary >= thisCenterFrequency && boundary < nextCenterFrequency)
	{
		filterParameter = (boundary - nextCenterFrequency) / (thisCenterFrequency - nextCenterFrequency);
		filterParameter *= GetMagnitudeFactor(filterBand);
	}
	else if(boundary >= nextCenterFrequency && boundary < samplingRate)
	{
		filterParameter = 0.0f;
	}

	return filterParameter;
}

/* 
 * Compute the band-dependent magnitude factor for the given filter band (Eq. 3)
 * Used for internal computation only - not the be called directly
 */
float32_t GetMagnitudeFactor(unsigned int filterBand)
{
	float32_t magnitudeFactor = 0.0f;

	if(filterBand >= 1 && filterBand <= 14)
	{
		magnitudeFactor = 0.015;
	}
	else if(filterBand >= 15 && filterBand <= 48)
	{
		magnitudeFactor = 2.0f / (GetCenterFrequency(filterBand + 1) - GetCenterFrequency(filterBand -1));
	}

	return magnitudeFactor;
}

/*
 * Compute the center frequency (fc) of the specified filter band (l) (Eq. 4)
 * This where the mel-frequency scaling occurs. Filters are specified so that their
 * center frequencies are equally spaced on the mel scale
 * Used for internal computation only - not the be called directly
 */
float32_t GetCenterFrequency(unsigned int filterBand)
{
	float32_t centerFrequency = 0.0f;
	float32_t exponent;

	if(filterBand == 0)
	{
		centerFrequency = 0;
	}
	else if(filterBand >= 1 && filterBand <= 14)
	{
		centerFrequency = (200.0f * filterBand) / 3.0f;
	}
	else
	{
		exponent = filterBand - 14.0f;
		centerFrequency = powf(1.0711703, exponent);
		centerFrequency *= 1073.4;
	}

	return centerFrequency;
}
