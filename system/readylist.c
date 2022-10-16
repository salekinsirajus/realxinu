/* Filename: readylist.c
 * Author: Shawn Salekin
 * Description:
 * 	Prints all process ID's in the ready list
 *
 * */
#include <xinu.h>

void print_queue(qid16 q, char *name){
    qid16 ptr = firstid(q);
	struct procent *prptr;

	sync_printf("%s: [", name);
    while (ptr != queuetail(q)){
		prptr = &proctab[ptr];
       	sync_printf("%d(%d), ", ptr, prptr->prstate); 
        ptr = queuetab[ptr].qnext;
    }  
	sync_printf("]\n");
}

syscall print_ready_list(){
	/* prints the contents of the readylist. 
	 * WORKS FINE
	 * */
	print_queue(readylist, "readyl");

    return OK;
}
