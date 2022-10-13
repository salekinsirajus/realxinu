/* Filename: readylist.c
 * Author: Shawn Salekin
 * Description:
 * 	Prints all process ID's in the ready list
 *
 * */
#include <xinu.h>

syscall print_ready_list(){
	/* prints the contents of the readylist. 
	 * WORKS FINE
	 * */

    qid16 ptr = firstid(readylist);

    while (ptr != queuetail(readylist)){
        sync_printf("%d prev: %d, next: %d\n", ptr, queuetab[ptr].qprev, queuetab[ptr].qnext); 
        ptr = queuetab[ptr].qnext;
    }  

    return OK;
}
