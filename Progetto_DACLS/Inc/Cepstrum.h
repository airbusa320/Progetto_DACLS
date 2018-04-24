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
float32_t prestato[2049];

void tFiltInit();

void powerSpectrum(float32_t * pIn, uint16_t len, float32_t * pOut);
void estrazione(float32_t * pIn, float32_t * pOut, uint16_t inLen, uint16_t nFilt, ftri_handle * filtri);
void estrazione2(float32_t * pIn, float32_t * pOut, uint16_t inLen, uint16_t nFilt, uint16_t nMFFC, ftri_handle * filtri);
void parametriDCT();



float32_t deviazione_standard_inv[80]={
		0.0202746698649018,
		0.146373869418845,
		0.101094323374649,
		0.142767087171608,
		0.1929032035524,
		0.314087025154721,
		0.288481633944991,
		0.342453301662918,
		0.33400652722812,
		0.381697259386625,
		0.417787534079739,
		0.480150138499654,
		0.489145580352376,
		0.565923256977463,
		0.609580194061792,
		0.637312795337896,
		0.668545439663886,
		0.72894291473307,
		0.733212695897081,
		0.807267343080983,
		0.0200887311555113,
		0.142163450222944,
		0.10352925750861,
		0.130265841697043,
		0.201236888644518,
		0.310670844099056,
		0.286416999344513,
		0.331357184581332,
		0.385398657928534,
		0.384371844099669,
		0.41899330618299,
		0.489511854104763,
		0.46724487649766,
		0.589378232062314,
		0.594763792088827,
		0.630795682656371,
		0.676082782383535,
		0.717451626841352,
		0.785392385916168,
		0.7888802652507,
		0.0200195105917587,
		0.151158160954403,
		0.109383974494987,
		0.135345797323871,
		0.204722509331429,
		0.275254257042593,
		0.236994927749473,
		0.324578801696107,
		0.347394674943238,
		0.342285509894591,
		0.428187388743409,
		0.494178681305423,
		0.460818793891486,
		0.572429226385358,
		0.594314856638666,
		0.611638235351806,
		0.638544820115902,
		0.701317324258674,
		0.761077465320321,
		0.811392722176176,
		0.0205755670792183,
		0.151925778660981,
		0.103404395980646,
		0.140968016426659,
		0.207332452522629,
		0.327481387655284,
		0.297994085310078,
		0.315469885941157,
		0.331745646569221,
		0.363276908236963,
		0.379794306577011,
		0.469419386869006,
		0.481228901150885,
		0.55693999984066,
		0.505057554807537,
		0.570287961630043,
		0.616731912144278,
		0.695824938223886,
		0.727545091430045,
		0.755271367935184,
};

float32_t media[80] = {
		-362.732879638671,
		0.279490172863007,
		-9.28191947937011,
		-2.04671502113342,
		0.561201572418212,
		-2.46985244750976,
		8.97079277038574,
		-2.36531138420104,
		3.16950154304504,
		2.31967830657958,
		1.42557144165039,
		0.859925806522369,
		0.203839614987373,
		0.479691714048386,
		1.19446158409118,
		-0.0366119518876076,
		0.345987379550934,
		0.0442263595759869,
		0.33283931016922,
		0.546532928943634,
		-360.415008544921,
		0.883649289608001,
		-10.1742095947265,
		-2.44215965270996,
		3.05144762992858,
		-2.9692690372467,
		7.39110136032104,
		-1.06510186195373,
		2.7575876712799,
		2.93557333946228,
		1.0871992111206,
		-0.0331040360033512,
		1.22133314609527,
		0.403814762830734,
		1.18514049053192,
		-0.0345370732247829,
		0.0365258529782295,
		0.49114653468132,
		0.203312650322914,
		0.293592572212219,
		-340.527954101562,
		0.326146513223648,
		-9.07288265228271,
		0.0912772938609123,
		3.16062498092651,
		-0.591340720653533,
		9.9211778640747,
		-0.970264196395874,
		2.99354362487792,
		1.87333512306213,
		0.0247892290353775,
		0.12721885740757,
		0.855376839637756,
		0.665803134441375,
		1.3095086812973,
		-0.185036316514015,
		0.302372843027115,
		0.353705614805222,
		0.23917019367218,
		0.333839297294617,
		-345.443328857421,
		-4.51070117950439,
		-14.5715856552124,
		-5.69514799118041,
		-2.74677324295043,
		-6.48737812042236,
		4.76302480697631,
		-4.56041097640991,
		1.58178865909576,
		2.15926313400268,
		0.665197789669036,
		-0.0890393182635307,
		-0.139029055833817,
		-0.178543090820312,
		1.06286346912384,
		-0.128794848918915,
		0.199574559926987,
		0.124268710613251,
		0.120543129742146,
		0.390305429697037
};

#endif /* CEPSTRUM_H_ */
