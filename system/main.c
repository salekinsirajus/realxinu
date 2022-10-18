/*  main.c  - main */

#include <xinu.h>
#include <stdio.h>

void sync_printf(char *fmt, ...)
{
    intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
}


void timed_execution(uint32 runtime){
            while(proctab[currpid].runtime<runtime);
}

void compute(uint32 runtime, uint32 *value)
{
        int i;
        while (proctab[currpid].runtime<runtime) {
            for (i = 0; i < 1000; i++)
                ;
            (*value)++;
        }
}

int main() {
        pid32 prA, prB;

        sync_printf("\n");
        sync_printf("=== TESTCASE 2::  2 processes with burst execution - context switches ===\n");
        prA = create_user_process(burst_execution, 1024, "burst_execution", 3, 4, 40, 40);
        prB = create_user_process(burst_execution, 1024, "burst_execution", 3, 4, 40, 40);
        set_tickets(prA, 90);
        set_tickets(prB, 10);
        resume(prA);
        resume(prB);
        receive();
        receive();
        sleepms(50); //wait for user processes to terminate
        kprintf("\nprocess %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        sync_printf("=========================================================================\n\n");

        return OK;
}
