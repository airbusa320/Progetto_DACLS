/****************************************************************
 *  															*
 *   Cepstrum.h												*
 * 																*
 *    Created on: 24 mar 2018									*
 *        Author: Riccardo, Stefano								*
 *	 															*
 ****************************************************************/

#ifndef CEPSTRUM_H_
#define CEPSTRUM_H_



#include "stm32f4xx_hal.h"
#include "main.h"

#define ARM_MATH_CM4
#include "arm_math.h"

#define FSTART 	0			// Parametri per il calcolo dei
#define FSTOP 	16000		// coefficienti MFCC
#define FS 		32000		//
#define NFFT 	1024		//
#define NFILT 	26			//
#define NMFCC 	20			//



float32_t pari[NFFT], dispari[NFFT];
float32_t DCTParam[NFILT*NFILT];

/* Struttura per l'handle dei filtri mel,
 * fstart: bin di inizio del filtro
 * len: lunghezza in bin del filtro
 * fdata: puntatore al primo bin del filtro memorizzato
 */
typedef struct {
	uint16_t fstart;
	uint16_t len;
	float32_t * fdata;
}ftri_handle;

ftri_handle hfilt[NFILT];

arm_fir_instance_f32 prem;
float32_t prestato[1025];

void tFiltInit();

void powerSpectrum(float32_t * pIn, uint16_t len, float32_t * pOut);
void estrazione(float32_t * pIn, float32_t * pOut, uint16_t inLen, uint16_t nFilt, ftri_handle * filtri);
void estrazione2(float32_t * pIn, float32_t * pOut, uint16_t inLen, uint16_t nFilt, uint16_t nMFFC, ftri_handle * filtri);
void parametriDCT();

#endif /* CEPSTRUM_H_ */
