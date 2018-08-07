#include <hls_math.h>
#include <sds_lib.h>
//#include <math.h>
#include "calc.h"



//#pragma SDS data copy(phase[0:CHANNEL_COUNT])
#pragma SDS data zero_copy(phase[0:CHANNEL_NUM_PE])
#pragma SDS data sys_port(phase:AFI)
#pragma SDS data access_pattern(phase:SEQUENTIAL)
void CalculatePhase(ap_uint<8>   *phase, float T_X, float T_Y, float T_Z, float Wavelength, int offset)
{
	float Delay;
	float Xdist_sq;
	float Ydist_sq;
	float zInt = T_Z;
	float Zdist_sq = (zInt - Height) * (zInt - Height);
	ap_uint<8>  phase_int;

	//#pragma HLS pipeline
	for(int i=offset; i< offset+CHANNEL_NUM_PE; i++){
		#pragma HLS pipeline
		Xdist_sq = T_X - XTransducerPositions[i];
		Ydist_sq = T_Y - YTransducerPositions[i];
		Xdist_sq = Xdist_sq * Xdist_sq;
		Ydist_sq = Ydist_sq * Ydist_sq;
		Delay = sqrtf( Xdist_sq + Ydist_sq + Zdist_sq) / 1000;
		Delay = 1-(mod(Delay,Wavelength) / Wavelength);
		Delay =  (Delay * 256);
		phase_int = (ap_uint<8>)(roundf(Delay));
		if (phase_int > 255){
			phase[i] = 0;
		}
		else
		{
			phase[i] = phase_int;
		}
	}
}

float mod(float numerator, float divisor){
	if (divisor > numerator)
		return numerator;
	else if (divisor == numerator)
		return 0;
	float remainder=0;
	float integer =0;
	remainder = modff(numerator/divisor,&integer);
	return remainder * divisor;
}


void CalculatePhaseX(ap_uint<8>  *phase, float T_X, float T_Y, float T_Z, float Wavelength)
{

	#pragma SDS resource(1)
    #pragma SDS async(1)
	CalculatePhase(phase,T_X,T_Y,T_Z,Wavelength,0);
	#pragma SDS resource(2)
	#pragma SDS async(2)
	phase = phase + CHANNEL_NUM_PE;
	CalculatePhase(phase,T_X,T_Y,T_Z,Wavelength,CHANNEL_NUM_PE);

	/*#pragma SDS resource(3)
	#pragma SDS async(3)
	phase = phase + CHANNEL_COUNT*2;
	CalculatePhase(phase,66,66,100,Wavelength,CHANNEL_COUNT,CHANNEL_COUNT);
	#pragma SDS resource(4)
	#pragma SDS async(4)
	phase = phase + CHANNEL_COUNT*3;
	CalculatePhase(phase,66,66,100,Wavelength,CHANNEL_COUNT,CHANNEL_COUNT);*/
	#pragma SDS wait(1)
	#pragma SDS wait(2)
	/*#pragma SDS wait(3)
	#pragma SDS wait(4)*/
}
