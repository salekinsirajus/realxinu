/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	highpq;			    /* Index of highpq ready list */
qid16	lowpq;			    /* Index of lowpq  ready list */

/*------------------------------------------------------------------------
 *  ready  -  Make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
status	ready(
	  pid32		pid		/* ID of process to make ready	*/
	)
{
	register struct procent *prptr;

	if (isbadpid(pid)) {
		return SYSERR;
	}

	/* Set process state to indicate ready and add to ready list */

	prptr = &proctab[pid];
	prptr->prstate = PR_READY;
	if (prptr->user_process){
		//insert(pid, highpq, prptr->prprio);
		enqueue(pid, highpq);
	} else {
		insert(pid, readylist, prptr->prprio);
	}
	resched();

	return OK;
}
