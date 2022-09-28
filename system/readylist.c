/* Filename: readylist.c
 * Author: Shawn Salekin
 * Description:
 * 	Prints all process ID's in the ready list
 *
 * */
#include <xinu.h>

syscall print_ready_list(){
    struct procent *prptr;
    int32 i;

    printf("\nProcesses that are in the readylist\n");
	for (i = 0; i < NPROC; i++) {
		prptr = &proctab[i];
		if (prptr->prstate == PR_FREE) {  /* skip unused slots	*/
			continue;
		}

		if (prptr->prstate == PR_READY) {
            printf("%l ", i);
        }

    }
    printf("\n");

    return OK;
}
