/*
 * VAD.c
 *
 *      Author: Stefano,Riccardo
 */

#include "VAD.h"
#define KEEP_ACTIVE 1

float32_t energy_frame;
uint8_t flag = 0;
float32_t Emax = 0;
float32_t Emin = 0;
float32_t threshold = 0;
float32_t delta = 1;
float32_t deltamax = 1;
float32_t deltadecay = 1.0005;
float32_t deltamaxdecay = 1.0001;
float32_t factor = 0.97;
marker mark;

uint8_t inactive_count = KEEP_ACTIVE;


marker VAD_AE(float32_t* frame,uint16_t frame_length/*, uint8_t i*/ )
{
	arm_rms_f32(frame, frame_length, &energy_frame);

	if (flag == 0){
		Emax = energy_frame;
		flag = 1;
		Emin = Emax;     //da rivedere
	}

	if (energy_frame > Emax){
		Emax = energy_frame;
		deltamax = 1;
	}else{
		deltamax=deltamax/deltamaxdecay;
	}

	if (energy_frame < Emin){

		if (energy_frame == 0){
			Emin = Emax;		//da rivedere
		}else{
			Emin = energy_frame;
		}
		delta = 1;
	}else{
		delta = delta*deltadecay;
	}
	factor = (Emax - Emin)/Emax;
	//factor_arr[i] = factor;

	threshold = ((1-factor)*Emax+(factor*Emin))*1.1;

	//threshold_arr[i]= threshold;
	//energy_frame_arr[i]= energy_frame;

	if (energy_frame > threshold){
		mark = ATTIVO;
		inactive_count = 0;}
	else{
		if (inactive_count >= KEEP_ACTIVE)
			mark = NON_ATTIVO;

		else{
			mark = ATTIVO;
			inactive_count++;
		}
	}
	Emax = Emax*deltamax;
	Emin = Emin*delta;
	//Emin_arr[i] = Emin;
	return mark;
}
