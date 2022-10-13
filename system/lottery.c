/* Implements a lottery scheduling for XINU
 * */
#include <xinu.h>
#include <stdlib.h>

void set_tickets(pid32 pid, uint32 tickets){
	srand(1);
	struct procent *pr;
    if (!pr->user_process){
        return;
    }
	kprintf("current tickets: %d\n", pr->prprio);
	// do more stuff as needed to convert tickets into priorities
	pr->prprio = tickets;
	kprintf("updated tickets: %d\n", pr->prprio);

}
