/****************************************************************
 *  															*
 *   Rete_MLP.c													*
 * 																*
 *    Created on: 16 mar 2018									*
 *        Author: Riccardo, Stefano								*
 *	 															*
 ****************************************************************/

#ifndef RETE_MLP_H_
#define RETE_MLP_H_



#include "stm32f4xx_hal.h"
#define ARM_MATH_CM4
#include "arm_math.h"



#define NLAYER 4

typedef enum
{
	TUTTO_OK=0,
	INIT_ERROR=1,
	TUTTO_ROTTO=3

} mlp_status;

typedef enum
{
	error=-1,
	UNO=0,
	DUE=1,
	TRE=2,
	QUATTRO=3,
	CINQUE=4,
	SEI=5,
	SETTE=6,
	OTTO=7,
	NOVE=8,
	DIECI=9,
	UNDICI=10
} event_flag;

typedef enum
{
	TANH=0,
	SOFTMAX=1,

} mlp_activation;


typedef struct {
	uint16_t len;
	uint16_t nin;
	mlp_activation activation;
	arm_matrix_instance_f32 pesi;
}mlp_handle;

mlp_handle hlayer[NLAYER];


mlp_status layer(mlp_handle* handle, float32_t *in, float32_t *out );

uint8_t MLP(float32_t* MFcc, uint8_t Lenght_MFcc, uint8_t i,uint8_t t,uint8_t l );

event_flag rete(float32_t * in);

void softmax(float32_t* in, float32_t* out,uint16_t len);

mlp_status rete_init();

event_flag maxind(float32_t * prob, uint16_t len);

const char* getEventName(event_flag eve);

#endif /* RETE_MLP_H_ */


