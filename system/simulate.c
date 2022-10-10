/*
simulate.c

Models the phase behavior of an application that alternates between a CPU and IO phase
*/
#include <xinu.h>

void burst_execution(uint32 number_bursts, uint32 burst_duration, uint32 sleep_duration){
	uint32 timer = 0;
	uint32 number_sleeps = number_bursts;
	bool8  go_sleep = 0;	/* flag to keep track of which cycle is next*/
	bool8  lock = 1;

	while (number_bursts + number_sleeps > 0){
		if (go_sleep){
			sleepms(sleep_duration);
			go_sleep = 0;
			number_sleeps--;
		} else {
			//simulate_busy_behavior
			//maybe use a compare and switch lock?
			timer = ctr1000 + burst_duration;
			while(1){
			   if (timer == ctr1000)break;	
			}
			number_bursts--;
			go_sleep = 1;
		}	
	}
}
