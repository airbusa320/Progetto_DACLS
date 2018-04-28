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
//#include "stdlib.h"

volatile uint16_t I2S_InternalBuffer[(I2Sbufflen*2)]={0}; //1ms(?) i2s buffer
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





void StartAcquisition()
{
	ITM->PORT[0].u8=33;
	ultimo=SX;
	HAL_I2S_Receive_DMA(&hi2s2,(uint16_t *)I2S_InternalBuffer,I2Sbufflen);
}


void AudioProcess(uint8_t *PDMBuf, uint8_t *PCMBuf)
{
	//PDM_Filter((uint16_t *)PDMBuf,(uint16_t *)PCMBuf,&hfiltro);

	PDM_Filter((PDMBuf),(PCMBuf),&PDM1_filter_handler);

}



void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s)
{
	//HAL_I2S_DMAPause(hi2s);



	uint32_t index = 0;

	volatile uint16_t * DataTempI2S = &(I2S_InternalBuffer[I2Sbufflen]);
	uint8_t a,b=0;
	for(index=0; index<I2Sbufflen; index++) {
		a = ((uint8_t *)(DataTempI2S))[(index*2)];
		b = ((uint8_t *)(DataTempI2S))[(index*2)+1];
		((uint8_t *)(PDMbuff))[(index*2)] = Channel_Demux[a & CHANNEL_DEMUX_MASK] | Channel_Demux[b & CHANNEL_DEMUX_MASK] << 4;;
		((uint8_t *)(PDMbuff))[(index*2)+1] = Channel_Demux[(a>>1) & CHANNEL_DEMUX_MASK] |Channel_Demux[(b>>1) & CHANNEL_DEMUX_MASK] << 4;

	}

	AudioProcess((uint8_t *)PDMbuff, (uint8_t *)PCMbuffsx);
	AudioProcess((uint8_t *)PDMbuff+1, (uint8_t *)PCMbuffdx);

	Process();


	//HAL_I2S_DMAResume(hi2s);
}



void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s)
{
	ITM->PORT[0].u8=34;
	//HAL_I2S_DMAPause(hi2s);
	uint32_t index = 0;

	volatile uint16_t * DataTempI2S = I2S_InternalBuffer;
	uint8_t a,b=0;
	for(index=0; index<I2Sbufflen; index++) {
		a = ((uint8_t *)(DataTempI2S))[(index*2)];
		b = ((uint8_t *)(DataTempI2S))[(index*2)+1];
		((uint8_t *)(PDMbuff))[(index*2)] = Channel_Demux[a & CHANNEL_DEMUX_MASK] | Channel_Demux[b & CHANNEL_DEMUX_MASK] << 4;;
		((uint8_t *)(PDMbuff))[(index*2)+1] = Channel_Demux[(a>>1) & CHANNEL_DEMUX_MASK] | Channel_Demux[(b>>1) & CHANNEL_DEMUX_MASK] << 4;

	}
	AudioProcess((uint8_t *)PDMbuff, (uint8_t *)PCMbuffsx);
	AudioProcess((uint8_t *)PDMbuff+1, (uint8_t *)PCMbuffdx);

	Process();

	ITM->PORT[0].u8=35;
	//HAL_I2S_DMAResume(hi2s);
}

void int2float(int16_t * psrc, float32_t * pDst, uint16_t size)
{
	int16_t * pin = psrc;
	uint16_t blkCnt;

	blkCnt = size >> 2u;

	while(blkCnt > 0u)
	{

		*pDst++ = ((float32_t)*pin++) / 32768;
		*pDst++ = ((float32_t)*pin++) / 32768;
		*pDst++ = ((float32_t)*pin++) / 32768;
		*pDst++ = ((float32_t)*pin++) / 32768;

		/* Decrement the loop counter */
		blkCnt--;
	}
	blkCnt = size % 0x4u;

	while(blkCnt > 0u)
	{

		*pDst++ = ((float32_t)*pin++) / 32768;

		/* Decrement the loop counter */
		blkCnt--;
	}
}


marker a;
float32_t bufferpspec[513],MFCC[80];
uint8_t stringa[10]={0};
uint8_t evento=0;
void Process()
{

	arm_copy_f32(framesx+512,framesx,512);	// sposta la metà destra dei buffer
	arm_copy_f32(framedx+512,framedx,512);	// nella metà sinistra
	int2float((int16_t *)PCMbuffsx,framesx+512,512);	// mette le nuove metà
	int2float((int16_t *)PCMbuffdx,framedx+512,512);	// nelle metà destre


	arm_add_f32(framedx,framesx,framesum,1024);
	a=VAD_AE(framesum,1024);


#ifdef VAD
	if (a==ATTIVO)
	{
#endif

		HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_SET);
		//HAL_Delay(1);

		arm_sub_f32(framedx,framesx,framediff,1024);

		powerSpectrum(framedx,NFFT,bufferpspec);
		estrazione2(bufferpspec,MFCC,513,NFILT,NMFCC,hfilt);
		powerSpectrum(framesx,NFFT,bufferpspec);
		estrazione2(bufferpspec,MFCC+20,513,NFILT,NMFCC,hfilt);
		powerSpectrum(framesum,NFFT,bufferpspec);
		estrazione2(bufferpspec,MFCC+40,513,NFILT,NMFCC,hfilt);
		powerSpectrum(framediff,NFFT,bufferpspec);
		estrazione2(bufferpspec,MFCC+60,513,NFILT,NMFCC,hfilt);

		arm_sub_f32(MFCC, (float32_t*)media, bufferpspec,80);
		arm_mult_f32(bufferpspec, (float32_t*)deviazione_standard_inv,MFCC,80);
		evento=(uint8_t)rete(MFCC);

		sprintf((char*)stringa,"%s",getEventName(evento));

		HAL_UART_Transmit(&huart2,stringa,sizeof(stringa),1000);

		HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_SET);

#ifdef VAD
	}
	else
	{
		HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_RESET);
	}
#endif




}


