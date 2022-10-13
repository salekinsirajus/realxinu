/* Filename: readylist.c
 * Author: Shawn Salekin
 * Description:
 * 	Prints all process ID's in the ready list
 *
 * */
#include <xinu.h>

syscall print_ready_list(){

    struct procent *prptr;
    pid32 ptr = firstkey(readylist);
    sync_printf("\------ readylist-----\n");
    while (ptr){
        sync_printf("%d ", ptr); 
        ptr = queuetab[ptr].qnext;
    }  
    sync_printf("\------end readylist-----\n");

    return OK;
}
