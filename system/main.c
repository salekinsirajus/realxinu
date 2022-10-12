/*  main.c  - main */

#include <xinu.h>

void sync_printf(char *fmt, ...)
{
    intmask mask = disable();
	void *arg = __builtin_apply_args();
	__builtin_apply((void*)kprintf, arg, 100);
	restore(mask);
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
