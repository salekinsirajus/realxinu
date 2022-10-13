/* Implements a lottery scheduling for XINU
 * */
#include <xinu.h>
#include <stdlib.h>

void set_tickets(pid32 pid, uint32 tickets){
	struct procent *pr;
    if (!pr->user_process){
        return;
    }

    //setting tickets here as prprio
	pr->prprio = tickets;
}
