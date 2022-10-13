/* resched.c - resched, resched_cntl */
#include <xinu.h>

#define DEBUG_CTXSW(o, n) printf("ctxsw::%d-%d\n", o, n);

struct	defer	Defer;

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

        /*
        tmp = &proctab[currpid]; 
        while(tmp->user_process){
            insert(currpid, readylist, tmp->prprio);
            currpid = dequeue(readylist);
            tmp = &proctab[currpid];
        }
*/
        /* Force context switch to highest priority ready process */

        pid32 newpid = currpid;
        ptnew = &proctab[currpid];
        ptnew->prstate = PR_CURR;
        ptnew->num_ctxsw += 1;
        preempt = QUANTUM;		/* Reset time slice for process	*/
        ctxsw(&ptold->prstkptr, &ptnew->prstkptr);
        if (oldpid != newpid){
            DEBUG_CTXSW(oldpid, newpid);
            if (old_process_is_curr){
                sync_printf("PID %d: Adding %d ms to runtime.\n", oldpid, ctr1000 - ptold->_rtstart);
                ptold->runtime += (ctr1000 - ptold->_rtstart);
                sync_printf("New runtime: %d\n", ptold->runtime);
                ptold->_rtstart = -1;
            }
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
            if (old_process_is_curr){
                sync_printf("PID %d: Adding %d ms to runtime.\n", oldpid, ctr1000 - ptold->_rtstart);
                ptold->runtime += (ctr1000 - ptold->_rtstart);
                sync_printf("New runtime: %d\n", ptold->runtime);
                ptold->_rtstart = -1;
            }
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
