#include <math.h>
#define initial_rto 250.0 //in ms

// Time units are calculated in  milliseconds

static float SRTT, RTTVAR;
static float g = .125, h = .25;
static double RTT = 250;

// Advantages of using statis so that the variables can be maintained 

double calc_rtt(struct timeval ack_time, struct timeval packet_time)
{
  double ack_time_in_mill = ((ack_time.tv_sec) * 1000) + ((ack_time.tv_usec) / 1000) ;
  double packet_time_in_mill = ((packet_time.tv_sec) * 1000) + ((packet_time.tv_usec) / 1000) ;
  RTT = (ack_time_in_mill - packet_time_in_mill);
  printf("RTT calculated for New sample :  -- is %.1f\n", RTT);
  return RTT;
}

void initialize_srtt_rttvar(float initial_RTT)
{

	SRTT = initial_RTT;
	RTTVAR = initial_RTT / 2;
	printf("Initial values: SRTT:%.1f \t RTTVAR: %.1f \n",SRTT,RTTVAR);
}

int computeRTO()
{
	float err, RTO;
	int rto;
	
	if(RTT == -1.0)
	{
		return initial_rto;
	}
	err = RTT - SRTT;
	SRTT = SRTT + g * err;
	RTTVAR = RTTVAR + h * (abs(err) - RTTVAR);
	RTO = SRTT + 4* RTTVAR;
	
	printf("--->>> UPDATED PARSMS : RTT: %.1f \t RTO: %.1f \tSRTT:%.1f \t RTTVAR: %.1f <<<----\n",RTT, RTO, SRTT,RTTVAR);
	//RTO = (rto > 1000 ? ((int)rto/1000)+3 : (int)3);		
	return (int)RTO;
}
