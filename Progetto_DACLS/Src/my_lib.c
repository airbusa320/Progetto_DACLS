/*
 * my_lib.c
 *
 *  Created on: 27 apr 2018
 *      Author: Riccardo
 */

#include "main.h"
#include "my_lib.h"
#include "pdm2pcm.h"
#include "VAD.h"
#include "Cepstrum.h"
#include "standard.h"
#include "Rete_MLP.h"
#include "tim.h"
//#include "stdlib.h"

volatile uint16_t I2S_InternalBuffer[(I2Sbufflen*2)]={0}; // Input buffer
volatile uint16_t PDMbuff[PDMbufflen]={0};
volatile uint16_t PCMbuffsx[PCMbufflen]={0};
volatile uint16_t PCMbuffdx[PCMbufflen]={0};
float32_t framesx[1024],framedx[1024],framesum[1024],framediff[1024];


const uint8_t Channel_Demux[128] = {
		0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x02, 0x03,
		0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x02, 0x03,
		0x04, 0x05, 0x04, 0x05, 0x06, 0x07, 0x06, 0x07,
		0x04, 0x05, 0x04, 0x05, 0x06, 0x07, 0x06, 0x07,
		0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x02, 0x03,
		0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x02, 0x03,
		0x04, 0x05, 0x04, 0x05, 0x06, 0x07, 0x06, 0x07,
		0x04, 0x05, 0x04, 0x05, 0x06, 0x07, 0x06, 0x07,
		0x08, 0x09, 0x08, 0x09, 0x0a, 0x0b, 0x0a, 0x0b,
		0x08, 0x09, 0x08, 0x09, 0x0a, 0x0b, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0c, 0x0d, 0x0e, 0x0f, 0x0e, 0x0f,
		0x0c, 0x0d, 0x0c, 0x0d, 0x0e, 0x0f, 0x0e, 0x0f,
		0x08, 0x09, 0x08, 0x09, 0x0a, 0x0b, 0x0a, 0x0b,
		0x08, 0x09, 0x08, 0x09, 0x0a, 0x0b, 0x0a, 0x0b,
		0x0c, 0x0d, 0x0c, 0x0d, 0x0e, 0x0f, 0x0e, 0x0f,
		0x0c, 0x0d, 0x0c, 0x0d, 0x0e, 0x0f, 0x0e, 0x0f
};

const uint8_t CHANNEL_DEMUX_MASK=0x55;




/*
 * Avvia la generazione del clock, attende un periodo di stabilizzazione
 * e avvia l'acquisizione dei dati dai microfoni tramite DMA
 */
void StartAcquisition()
{
	HAL_TIM_PWM_Start(&htim3,TIM_CHANNEL_2);
	HAL_Delay(250);
	HAL_I2S_Receive_DMA(&hi2s2,(uint16_t *)I2S_InternalBuffer,I2Sbufflen);
}


void PDM2PCM(uint8_t *PDMBuf, uint8_t *PCMBuf)
{
	PDM_Filter((PDMBuf),(PCMBuf),&PDM1_filter_handler);
}


/*
 * Funzione che viene chiamata quando il DMA riempie la seconda metà del buffer
 */
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	uint32_t index = 0;

	volatile uint16_t * DataTempI2S = &(I2S_InternalBuffer[I2Sbufflen]);
	uint8_t a=0,b=0;
	for(index=0; index<I2Sbufflen; index++) // deinterlacciamento bit
	{
		a = ((uint8_t *)(DataTempI2S))[(index*2)];
		b = ((uint8_t *)(DataTempI2S))[(index*2)+1];
		((uint8_t *)(PDMbuff))[(index*2)] = Channel_Demux[a & CHANNEL_DEMUX_MASK] | Channel_Demux[b & CHANNEL_DEMUX_MASK] << 4;;
		((uint8_t *)(PDMbuff))[(index*2)+1] = Channel_Demux[(a>>1) & CHANNEL_DEMUX_MASK] |Channel_Demux[(b>>1) & CHANNEL_DEMUX_MASK] << 4;
	}

	PDM2PCM((uint8_t *)PDMbuff, (uint8_t *)PCMbuffsx);
	PDM2PCM((uint8_t *)PDMbuff+1, (uint8_t *)PCMbuffdx);

	Process();
}


/*
 * Funzione che viene chiamata quando il DMA riempie la prima metà del buffer
 */
void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	uint32_t index = 0;

	volatile uint16_t * DataTempI2S = I2S_InternalBuffer;
	uint8_t a=0,b=0;
	for(index=0; index<I2Sbufflen; index++) // deinterlacciamento bit
	{
		a = ((uint8_t *)(DataTempI2S))[(index*2)];
		b = ((uint8_t *)(DataTempI2S))[(index*2)+1];
		((uint8_t *)(PDMbuff))[(index*2)] = Channel_Demux[a & CHANNEL_DEMUX_MASK] | Channel_Demux[b & CHANNEL_DEMUX_MASK] << 4;;
		((uint8_t *)(PDMbuff))[(index*2)+1] = Channel_Demux[(a>>1) & CHANNEL_DEMUX_MASK] | Channel_Demux[(b>>1) & CHANNEL_DEMUX_MASK] << 4;
	}

	PDM_Filter((uint8_t *)PDMbuff, (uint8_t *)PCMbuffsx, &PDM1_filter_handler);
	PDM_Filter((uint8_t *)PDMbuff+1, (uint8_t *)PCMbuffdx, &PDM1_filter_handler);

	Process();
}

void int2float(int16_t * psrc, float32_t * pDst, uint16_t size)
{
	int16_t * pin = psrc;
	uint16_t blkCnt;

	blkCnt = size >> 2u;

	while(blkCnt > 0u)
	{

		*pDst++ = ((float32_t)*pin++) / MAXINT16;
		*pDst++ = ((float32_t)*pin++) / MAXINT16;
		*pDst++ = ((float32_t)*pin++) / MAXINT16;
		*pDst++ = ((float32_t)*pin++) / MAXINT16;

		/* Decrement the loop counter */
		blkCnt--;
	}
	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{

		*pDst++ = ((float32_t)*pin++) / MAXINT16;

		/* Decrement the loop counter */
		blkCnt--;
	}
}

#ifdef VAD
marker a;
#endif

float32_t bufferpspec[513],MFCC[80];
uint8_t stringa[20]={0};
uint8_t evento=0;
uint8_t len;
void Process()
{

	arm_copy_f32(framesx+512,framesx,512);	// sposta le metà di destra dei
	arm_copy_f32(framedx+512,framedx,512);	// buffer nelle metà di sinistra
	int2float((int16_t *)PCMbuffsx,framesx+512,512);	// mette le nuove metà
	int2float((int16_t *)PCMbuffdx,framedx+512,512);	// nelle metà di destra


	arm_add_f32(framedx,framesx,framesum,1024);		// calcola il canale somma

#ifdef VAD
	a=VAD_AE(framesum,1024);	// valuta l'attivazione del frame

	if (a==ATTIVO)
	{
#endif

		HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_SET);	// LD2 on

		arm_sub_f32(framedx,framesx,framediff,1024);// calcola il canale differenza

		// estrazione delle feature dai 4 canali
		powerSpectrum(framedx,NFFT,bufferpspec);
		estrazione(bufferpspec,MFCC,513,NFILT,NMFCC,hfilt);

		powerSpectrum(framesx,NFFT,bufferpspec);
		estrazione(bufferpspec,MFCC+20,513,NFILT,NMFCC,hfilt);

		powerSpectrum(framesum,NFFT,bufferpspec);
		estrazione(bufferpspec,MFCC+40,513,NFILT,NMFCC,hfilt);

		powerSpectrum(framediff,NFFT,bufferpspec);
		estrazione(bufferpspec,MFCC+60,513,NFILT,NMFCC,hfilt);


		// standardizzazione feature
		arm_sub_f32(MFCC, (float32_t*)media, bufferpspec,80);
		arm_mult_f32(bufferpspec, (float32_t*)deviazione_standard_inv,MFCC,80);
		evento=(uint8_t)rete(MFCC);

		len=sprintf((char*)stringa,"%s\n",getEventName(evento));
		HAL_UART_Transmit(&huart2,stringa,len,1000);

#ifdef VAD
	}
	else
	{
		HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_RESET);	// LD2 off
	}
#endif




}


