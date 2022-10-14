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
        if (!prptr->user_process){
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
        if (prptr->user_process){
            //TODO: should be replaced with tickets
            ticket_sum += prptr->prprio;
        } 
        cursor = queuetab[cursor].qnext;
    }

    return ticket_sum;
}

pid32 lottery(){

    /* When there is system process in the readylist do not execute the user
     * process. If they are at the front, that's fine. But if they are in the
     * middle that's when things start to cause issues. How do we solve this?
     *
     * Should we use a different way to represent tickets intstead of priority?
     * What about adding a field called ticket? 
     * */



    int MAX_TICKETS = get_tickets_for_draw();
    if (MAX_TICKETS == 0) return dequeue(readylist); //do not hold the lottery
    int winner = rand() % MAX_TICKETS;
    int counter = 0;
    //sync_printf("winner is: %d, max_tickets: %d\n", winner, MAX_TICKETS); 

    struct procent *prptr;
    qid16 cursor = firstid(readylist);

    while (cursor != queuetail(readylist)){
        prptr = &proctab[cursor];
        if (prptr->user_process){
            //FIXME: tickets or priority
            counter += prptr->prprio;
            if (counter > winner){
                //sync_printf("found the winner %d with PID: %d\n", winner, cursor);
                return cursor; // should be a PID
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
    /*
     *
            if (old_process_is_curr){
                //sync_printf("PID %d: Adding %d ms to runtime.\n", oldpid, ctr1000 - ptold->_rtstart);
                ptold->runtime += (ctr1000 - ptold->_rtstart);
                //sync_printf("New runtime: %d\n", ptold->runtime);
                ptold->_rtstart = -1;
            }
     *
     * */
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
    //bool8 old_process_is_curr = (ptold->prstate == PR_CURR);
    bool8 old_process_is_curr = 1; 

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

        pid32 newpid = currpid;
        ptnew = &proctab[currpid];
        ptnew->prstate = PR_CURR;
        ptnew->num_ctxsw += 1;
        preempt = QUANTUM;		/* Reset time slice for process	*/
        ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
        if (oldpid != newpid){
            DEBUG_CTXSW(oldpid, newpid);
        }
    } else {
        ptold->prstate = PR_READY;
        insert(currpid, readylist, ptold->prprio);

        currpid = lottery();
        pid32 newpid = currpid;
        ptnew = &proctab[currpid];
        ptnew->prstate = PR_CURR;
        ptnew->num_ctxsw += 1;
        preempt = QUANTUM;		/* Reset time slice for process	*/
        ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
        if (oldpid != newpid){
            DEBUG_CTXSW(oldpid, newpid);
        }
    }

    ptnew->_rtstart = ctr1000;
	/* Old process returns here when resumed */

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
