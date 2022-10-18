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

void burst_execution(uint32 number_bursts, uint32 burst_duration, uint32 sleep_duration){
	uint32 i;

	uint32 timer;

	for (i=0; i < 2 * number_bursts; i++){
		timer = (burst_duration) + ctr1000;
		while (timer> ctr1000);
		sleepms(sleep_duration);
	}
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
        pid32 prA, prB, prC, prD;

        kprintf("\n");

        kprintf("QUANTUM=%d, TIME_ALLOTMENT=%d, PRIORITY_BOOST_PERIOD=%d\n\n", QUANTUM, TIME_ALLOTMENT, PRIORITY_BOOST_PERIOD);

        kprintf("=== TESTCASE 1::  CPU-intensive jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);

        resume(prA);
        resume(prB);

        receive();
        receive();

        sleepms(50); // wait for user processes to terminate

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 2::  CPU-intensive jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prC = create_user_process(timed_execution, 1024, "cpu_1000_new", 1, 1000);
        prD = create_user_process(timed_execution, 1024, "cpu_1000_del", 1, 1000);

        resume(prA);
        resume(prB);

        receive();
        receive();

        resume(prC);

        sleepms(500);

        resume(prD);

        receive();
        receive();

        sleepms(50); // wait for user processes to terminate

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 3::  interactive jobs =============================\n");

        prA = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prB = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prC = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prD = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);

        resume(prA);
        resume(prB);
        resume(prC);
        resume(prD);

        receive();
        receive();
        receive();
        receive();

        sleepms(50); // wait for user processes to terminate

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 4: mixed jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prC = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prD = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);

        resume(prA);
        resume(prB);
        resume(prC);
        resume(prD);

        receive();
        receive();
        receive();
        receive();

        sleepms(50); // wait for user processes to terminate

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 5::  mixed jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prC = create_user_process(burst_execution, 1024, "burst_100/2/8", 3, 100, 2, 8);
        prD = create_user_process(burst_execution, 1024, "burst_100/2/8", 3, 100, 2, 8);

        resume(prA);
        resume(prB);
        resume(prC);
        resume(prD);

        receive();
        receive();
        receive();
        receive();

        sleepms(50); // wait for user processes to terminate

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        kprintf("=== TESTCASE 6::  mixed jobs =============================\n");

        prA = create_user_process(timed_execution, 1024, "cpu_1000", 1, 1000);
        prB = create_user_process(burst_execution, 1024, "burst_100/1/9", 3, 100, 1, 9);
        prC = create_user_process(burst_execution, 1024, "burst_100/5/5", 3, 100, 5, 5);
        prD = create_user_process(burst_execution, 1024, "burst_20/40/10", 3, 20, 40, 10);

        resume(prA);
        resume(prB);
        resume(prC);
        resume(prD);

        receive();
        receive();
        receive();
        receive();

        sleepms(50); // wait for user processes to terminate

        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prA, proctab[prA].prname, proctab[prA].runtime, proctab[prA].turnaroundtime, proctab[prA].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prB, proctab[prB].prname, proctab[prB].runtime, proctab[prB].turnaroundtime, proctab[prB].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prC, proctab[prC].prname, proctab[prC].runtime, proctab[prC].turnaroundtime, proctab[prC].num_ctxsw);
        kprintf("process %d:: name=%s, runtime=%d, turnaround time=%d, ctx=%d\n",prD, proctab[prD].prname, proctab[prD].runtime, proctab[prD].turnaroundtime, proctab[prD].num_ctxsw);

        kprintf("==================================================================\n\n");

        return OK;

}
