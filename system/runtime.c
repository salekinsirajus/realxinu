/*
updates runtime between context switches and other OS events
*/
#include <xinu.h>
#include <stdio.h>

void calculate_runtime(pid32 pid){
	struct procent *prptr;
	prptr = &proctab[pid];
	uint32 last_runtime = 0;
    if (prptr->_rtstart != -1){
		last_runtime = ctr1000 - prptr->_rtstart;
	} 

	/*
	else {
		sync_printf("PID %d: _rtstart :%d, should be less than ctr1000\n", pid, prptr->_rtstart);
	}
	*/
	prptr->runtime += last_runtime;
	prptr->_rtstart = -1;
}

void start_runtime(pid32 pid){
	struct procent *prptr;
	prptr = &proctab[pid];

	/*
	if (prptr->_rtstart != -1){
		sync_printf("PID %d: _rtsart %d, should be -1\n", pid, prptr->_rtstart);
	}
	*/
	prptr->_rtstart = ctr1000;
}
