/*  main.c  - main */

#include <xinu.h>

void sync_printf(char *fmt, ...)
{
    intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
}

process burst_execution(uint32 number_bursts, uint32 burst_duration, uint32 sleep_duration){
	uint32 timer = 0;
	uint32 number_sleeps = number_bursts;
	bool8  go_sleep = 0;	/* flag to keep track of which cycle is next*/

	while (number_bursts + number_sleeps > 0){
        sync_printf("number_bursts + number_sleeps: %d, %d\n", number_bursts, number_sleeps);
		if (go_sleep){
            sync_printf("sleeping for %d\n", sleep_duration);
			sleepms(sleep_duration);
			go_sleep = 0;
			number_sleeps--;
		} else {
			//simulate_busy_behavior
			//maybe use a compare and switch lock?
            sync_printf(" <Busy> ");
			timer = ctr1000 + burst_duration;
			while(1){
               int a = 0;
               a = 9999999 * 9999999;
			   if (ctr1000 >= ctr1000)break;	
			}
			number_bursts--;
			go_sleep = 1;
		}	
        sync_printf("Getting into the while loop\n");
	}
    sync_printf("done executing this process with PID: %d\n", currpid);

    return OK;
}

process	main(void)
{
    pid32 p1 = create_user_process(burst_execution, 1094, "test1", 3, 10, 100, 100);
    set_tickets(p1, 2);
    resume(p1);
    pid32 p2 = create_user_process(burst_execution, 1094, "test2", 3, 5, 50, 50);
    set_tickets(p1, 1);
    resume(p2);
}
