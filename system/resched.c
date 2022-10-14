//TODO: stop context switching when there is only the null process
//in the system
/* resched.c - resched, resched_cntl */
#include <xinu.h>
#include <stdlib.h>

#define DEBUG_CTXSW(o, n) printf("ctxsw::%d-%d\n", o, n);
#define DEBUG_CTXSW2(a, b, c)	  printf("pid: %d, prio: %d, u/s: %d\n", a , b, c);

struct	defer	Defer;

bool8 skip_lottery(){
    /*If there is any process in the readylist that's a system process, we can't
     * use lottery to schedule user processes.*/

    struct procent *prptr;
    qid16 ptr = firstid(readylist);

    while (ptr != queuetail(readylist)){
        prptr = &proctab[ptr];
		if (!prptr->prprio > 1){
		//FIXME: what happens when there is only null process?
        //if (!prptr->user_process){
            return TRUE;
        }
        ptr = queuetab[ptr].qnext;
    }

    return FALSE;
}


int get_tickets_for_draw(){
    /* Iterate over user processes in the readylist to sum drawable tickets.
     * WORKING.
     * */
    //shortcut, if the list is empty, return 0
    if (isempty(readylist)){
        return 0;
    }

    qid16 cursor = firstid(readylist);
    struct procent *prptr;
    int ticket_sum = 0;

    while (cursor != queuetail(readylist)){
        prptr = &proctab[cursor];
        if ((prptr->user_process)){
			
            ticket_sum += prptr->tickets;
        } 
        cursor = queuetab[cursor].qnext;
    }

    return ticket_sum;
}

pid32 lottery(){

    int MAX_TICKETS = get_tickets_for_draw();
	//do not hold the lottery if there are still system process
    if (MAX_TICKETS == 0) return dequeue(readylist);

    struct procent *prptr;
    qid16 cursor = firstid(readylist);

    int winner = rand() % MAX_TICKETS;
    int counter = 0;
    while (cursor != queuetail(readylist)){
        prptr = &proctab[cursor];
        if (prptr->user_process && prptr->tickets > 0){
            counter += prptr->tickets;
            if (counter > winner){
                break; 
            }
        } 
        cursor = queuetab[cursor].qnext;
    }

    return getitem(cursor); 
}

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}


	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];
	pid32  oldpid = currpid;
	pid32  newpid;

    if (skip_lottery()){
        if (ptold->prstate == PR_CURR) {  /* Process remains eligible */
            if (ptold->prprio > firstkey(readylist)) {
                return;
            }

            /* Old process will no longer remain current */

            ptold->prstate = PR_READY;
            insert(currpid, readylist, ptold->prprio);
        }

        currpid = dequeue(readylist);
        /* Force context switch to highest priority ready process */

        newpid = currpid;
        ptnew = &proctab[currpid];
        ptnew->prstate = PR_CURR;
        preempt = QUANTUM;		/* Reset time slice for process	*/
        ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
        if (oldpid != newpid){
            DEBUG_CTXSW(oldpid, newpid);
        }
    } else {
		if (ptold->prstate == PR_CURR){
        	ptold->prstate = PR_READY;
        	insert(currpid, readylist, ptold->prprio);
		}

        currpid = lottery();
        newpid = currpid;
        ptnew = &proctab[currpid];
        ptnew->prstate = PR_CURR;
        preempt = QUANTUM;		/* Reset time slice for process	*/
        ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
        if (oldpid != newpid){
            DEBUG_CTXSW(oldpid, newpid);
        }
    }

    ptnew->num_ctxsw += 1;
	start_runtime(newpid);

	return;
}

/*------------------------------------------------------------------------
 *  resched_cntl  -  Control whether rescheduling is deferred or allowed
 *------------------------------------------------------------------------
 */
status	resched_cntl(		/* Assumes interrupts are disabled	*/
	  int32	defer		/* Either DEFER_START or DEFER_STOP	*/
	)
{
	switch (defer) {

	    case DEFER_START:	/* Handle a deferral request */

		if (Defer.ndefers++ == 0) {
			Defer.attempt = FALSE;
		}
		return OK;

	    case DEFER_STOP:	/* Handle end of deferral */
		if (Defer.ndefers <= 0) {
			return SYSERR;
		}
		if ( (--Defer.ndefers == 0) && Defer.attempt ) {
			resched();
		}
		return OK;

	    default:
		return SYSERR;
	}
}
