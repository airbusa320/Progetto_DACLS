/****************************************************************
 *  															*
 *   Rete_MLP.c													*
 * 																*
 *    Created on: 16 mar 2018									*
 *        Author: Riccardo, Stefano								*
 *	 															*
 ****************************************************************

 ================================================================
                     ##### Rete Neurale #####
 ================================================================

 Modificare la definizione del numero di layer NLAYER e modificare
 in accordo alle proprie esigenze le funzioni "rete_init", "rete",
 e l'enum con le flag delle uscite.

 Chiamare la funzione "rete_init" per generare gli handle dei vari
 layer della rete e usare la funzione "rete" per il calcolo delle
 uscite.
 */


#include "Rete_MLP.h"

#include "main.h"
#include "math.h"


#include "arm_const_structs.h"

#include "Weight_bias.h"
//#include "tim.h"

// Layer 64 percettroni

//float32_t bias[192];
/*uint8_t size_net = 64;
uint8_t size_out = 11;
float32_t In_perc[192];
float32_t sum[192];
float32_t Y_1[64];
float32_t Y_2[64];
float32_t Y_3[64];
float32_t Y_Out[11];
float32_t SM_Out[11];

uint8_t MLP(float32_t* MFcc, uint8_t Lenght_MFcc, uint8_t i,uint8_t t,uint8_t l )
{
	HAL_TIM_Base_Start(&htim2);
	for(i = 0; i <= size_net; i++)
	{

		arm_dot_prod_f32(MFcc,(Weight_array+i*(Lenght_MFcc+1)+1),Lenght_MFcc,&In_perc[i]);
		sum[i] = In_perc[i] + Weight_array[(Lenght_MFcc+1)*i];
		Y_1[i] = tanhf(sum[i]);

	}
	uint16_t ini_l = (Lenght_MFcc+1)*size_net ;
	for(l = 0; l <= size_net; l++)
	{

		arm_dot_prod_f32(Y_1,(Weight_array+l*(size_net+1)+1+ini_l),size_net,&In_perc[l+64]);
		sum[l+64] = In_perc[l+64] + Weight_array[(size_net+1)*l+ini_l];
		Y_2[l] = tanhf(sum[l+64]);

	}
	uint16_t ini_t = ini_l+(size_net+1)*size_net ;
	for(t = 0; t <= size_net; t++)
	{

		arm_dot_prod_f32(Y_2,(Weight_array+t*(size_net+1)+1+ini_t),size_net,&In_perc[t+128]);
		sum[t+128] = In_perc[t+128] + Weight_array[(size_net+1)*t+ini_t];
		Y_3[t] = tanhf(sum[t+128]);

	}


	uint16_t ini_q = ini_t+(size_net+1)*size_net ;
	for(int q = 0; q <= size_out; q++)
	{

		arm_dot_prod_f32(Y_3,(Weight_array+q*(size_net+1)+1+ini_q),size_net,&In_perc[q+128+64]);
		Y_Out[q] = In_perc[q+128+64] + Weight_array[(size_net+1)*q+ini_q];
		//Y_3[q] = tanh(sum[q+128]);
	}


	HAL_TIM_Base_Stop(&htim2);

	uint32_t cacca=__HAL_TIM_GetCounter(&htim2);

	HAL_TIM_Base_Init(&htim2);
	return 0;
}*/

/*
 * Inizializza gli handle dei layer
 */
mlp_status rete_init()
{
	hlayer[0].activation=TANH;
	hlayer[0].len=64;
	hlayer[0].nin=80;
	hlayer[0].pesi.numCols=hlayer[0].nin+1;
	hlayer[0].pesi.numRows=hlayer[0].len;
	hlayer[0].pesi.pData=(float32_t *)Weight_array;

	hlayer[1].activation=TANH;
	hlayer[1].len=64;
	hlayer[1].nin=64;
	hlayer[1].pesi.numCols=hlayer[1].nin+1;
	hlayer[1].pesi.numRows=hlayer[1].len;
	hlayer[1].pesi.pData=hlayer[0].pesi.pData+((hlayer[0].nin+1)*hlayer[0].len);

	hlayer[2].activation=TANH;
	hlayer[2].len=64;
	hlayer[2].nin=64;
	hlayer[2].pesi.numCols=hlayer[2].nin+1;
	hlayer[2].pesi.numRows=hlayer[2].len;
	hlayer[2].pesi.pData=hlayer[1].pesi.pData+((hlayer[1].nin+1)*hlayer[1].len);

	hlayer[3].activation=SOFTMAX;
	hlayer[3].len=11;
	hlayer[3].nin=64;
	hlayer[3].pesi.numCols=hlayer[3].nin+1;
	hlayer[3].pesi.numRows=hlayer[3].len;
	hlayer[3].pesi.pData=hlayer[2].pesi.pData+((hlayer[2].nin+1)*hlayer[2].len);

	return TUTTO_OK;

}

float32_t buffer[200];

/* calcola l'uscita del layer
 * handle: puntatore al handle
 * in: puntatore al vettore degli ingressi
 * out: puntatore al vettore delle uscite
 */
mlp_status layer(mlp_handle* handle, float32_t *in, float32_t *out )
{
	// per sfruttare l'implementazione ARM del calcolo matriciale
	// converte i vettori in e out in matrici di una colonna

	arm_matrix_instance_f32 inmat, outmat;
	inmat.numRows=handle->nin+1;
	inmat.numCols=1;
	buffer[0]=1.0f;			// ingresso unitario del bias
	arm_copy_f32(in,buffer+1,handle->nin);
	inmat.pData=buffer;

	outmat.numRows=handle->len;
	outmat.numCols=1;
	outmat.pData=out;

	if (arm_mat_mult_f32(&(handle->pesi),&inmat,&outmat) != ARM_MATH_SUCCESS)
		// calcolo della somma pesata
	{
		return TUTTO_ROTTO;	// se la moltiplicazione restituisce un errore
	}


	switch (handle->activation) {	// calcolo dell'attivazione
	case TANH:
		for (int var = 0; var < ((outmat.numCols)*(outmat.numRows)); ++var)
		{
			(out[var])=tanhf((out[var]));
		}
		break;
	case SOFTMAX:
		softmax(out,out,handle->len);
		break;
	default:
		return TUTTO_ROTTO;
	}

	return TUTTO_OK;
}


event_flag rete(float32_t * in)
{
	float32_t temp[200],temp2[200]; // buffer provvisori

	if (layer(&hlayer[0],in,temp)!= TUTTO_OK)
	{
		return -1;
	}

	if (layer(&hlayer[1],temp,temp2)!= TUTTO_OK)
	{
		return -1;
	}

	if (layer(&hlayer[2],temp2,temp)!= TUTTO_OK)
	{
		return -1;
	}

	if (layer(&hlayer[3],temp,temp2)!= TUTTO_OK)
	{
		return -1;
	}

	return maxind(temp2,11);
}


/* restituisce l'indice dell'evento identificato
 * prob: puntatore al vettore delle probabilità calcolate dal layer di uscita
 * len: lunghezza del vettore
 */
event_flag maxind(float32_t * prob, uint16_t len)
{
	uint16_t temp=0; // indice del massimo provvisorio
	for (int i = 1; i < len; ++i) {
		if (*(prob+i)>*(prob+temp))
		{
			temp=i;
		}
	}
	return (event_flag)temp;
}


/*
 * calcolo della funzione softmax
 */
void softmax(float32_t* in, float32_t* out,uint16_t len)
{
	float32_t  temp=0;
	for (int i = 0; i < len; ++i) {
		out[i]=expf(in[i]);
		temp+=out[i];
	}
	arm_scale_f32(out,(1.0f)/temp,out,len);
}


const char* getEventName(event_flag eve)
{
   switch (eve)
   {
      case UNO: return "Uno";
      case DUE: return "Due";
      case TRE: return "Tre" ;
      case QUATTRO: return "Quattro" ;
      case CINQUE: return "Cinque" ;
      case SEI: return "Sei" ;
      case SETTE: return "Sette" ;
      case OTTO: return "Otto" ;
      case NOVE: return "Nove" ;
      case DIECI: return "Dieci" ;
      case UNDICI: return "Undici" ;
      default: return "rotto";
   }


}


