/* ready.c - ready */

#include <xinu.h>

qid16	readylist;			/* Index of ready list		*/
qid16	highpq;			    /* Index of highpq ready list */
qid16	midpq;			    /* Index of midpq ready list */
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

	/* We add a process to the highpq the firt time it is scheduled 
	 * if prptr->runtime == 0, it has not been run.
     */
	if (prptr->user_process){
		//enqueue(pid, highpq);
		enqueue_mlfq(pid);
	} else {
		insert(pid, readylist, prptr->prprio);
	}
	resched();

	return OK;
}
