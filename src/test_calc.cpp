#include <stdio.h>
#include <ap_int.h>
#include "calc.h"
#include "sds_lib.h"


class perf_counter
{
public:
     uint64_t tot, cnt, calls;
     perf_counter() : tot(0), cnt(0), calls(0) {};
     inline void reset() { tot = cnt = calls = 0; }
     inline void start() { cnt = sds_clock_counter(); calls++; };
     inline void stop() { tot += (sds_clock_counter() - cnt); };
     inline uint64_t avg_cpu_cycles() { return ((tot+(calls>>1)) / calls); };
};

//Software version of CalculatePhase: Calculate the phase delays for a target position
void CalculatePhase_SW(int *phase, float T_X, float T_Y, float T_Z, float Wavelength){

	double ChannelDelays[CHANNEL_NUM] ={0};
	int i=0;
	for(i=0; i< CHANNEL_NUM; i++){

		// Calculate distance between transducer elements to focal point
		ChannelDelays[i] = sqrt(pow((T_X - XTransducerPositions[i]),2) + pow((T_Y - YTransducerPositions[i]),2) + pow((T_Z - Height),2)) / 1000;

		// Phase shift value equal to remainder of distance divided by wavelength. Expressed as fraction of wavelength.
		ChannelDelays[i] = 1-(fmod(ChannelDelays[i],Wavelength) / Wavelength);

		// Convert final phase shift values into 8 bit numbers
		ChannelDelays[i] =  (ChannelDelays[i] * 256)  ;

		phase[i] = (ap_uint<8>)(roundf(ChannelDelays[i]));
				if (phase[i] > 255){
					phase[i] = 0;
				}
	}

	return;
}



int main(){
	perf_counter hw_ctr, sw_ctr;
	printf("Testing core\n");
	ap_uint<8> *phase;
	int  phase_sw[CHANNEL_NUM];
	float Wavelength = 0.00850725;
	phase = (ap_uint<8> *)sds_alloc_non_cacheable(CHANNEL_NUM * sizeof(ap_uint<8>));

	hw_ctr.start();
	for(int i=0;i<10000;i++)
	{
		CalculatePhaseX(phase,i,66,100,Wavelength);
	}
	hw_ctr.stop();

	sw_ctr.start();
	for(int i=0;i<10000;i++)
	{
		CalculatePhase_SW(phase_sw,i,66,100,Wavelength);
	}
	sw_ctr.stop();

	uint64_t sw_cycles = sw_ctr.avg_cpu_cycles();
	uint64_t hw_cycles = hw_ctr.avg_cpu_cycles();
	double speedup = (double) sw_cycles / (double) hw_cycles;

	std::cout << "Average number of CPU cycles running CalculatePhase in software: "
	               << sw_cycles << std::endl;
	std::cout << "Average number of CPU cycles running CalculatePhase in hardware: "
	               << hw_cycles << std::endl;
	std::cout << "Speed up: " << speedup << std::endl;

	printf("HW Phase is: %d %d %d %d\n",(int)phase[0],(int)phase[12],(int)phase[24],(int)phase[56]);

	printf("SW Phase is: %d %d %d %d\n",(int)phase_sw[0],(int)phase_sw[12],(int)phase_sw[24],(int)phase_sw[56]);
/*
	for(int i=0; i< CHANNEL_NUM; i++){
		printf("Phase is: %d\n",(int)phase[i]);
	}*/
	return 0;


}

