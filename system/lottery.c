/* Implements a lottery scheduling for XINU
 * */
#include <xinu.h>
#include <stdlib.h>

void set_tickets(pid32 pid, uint32 tickets){
	struct procent *pr;
    pr = &proctab[pid];
    if (!pr->user_process){
        return;
    }

    //setting tickets to thw process
	pr->tickets = tickets;
    //sync_printf("Adding %d tickets to PID %d, new value: %d\n", tickets, pid, pr->tickets);

    return;
}
