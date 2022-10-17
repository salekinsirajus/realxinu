/* resched.c - resched, resched_cntl */

#include <xinu.h>

#define DEBUG_CTXSW(o, n) printf("ctxsw::%d-%d\n", o, n);

struct	defer	Defer;

bool8 readylist_has_only_nullprocess(){
	if (firstid(readylist) == 0){
		return TRUE;
	}
	return FALSE;
}

bool8 readylist_has_nonnull_sys_process(){
    /*If there is any process in the readylist that's a system process, we can't
     * use lottery to schedule user processes.*/
	if (isempty(readylist)){
		return FALSE;
	}
	//We are assuming ANY entry in the readylist is a system process
	//so if the queue is empty or there is only one process and it's 
	//null process it's okay to run mlfq, otherwise don't.
	//this also based on the invariant the nullprocess will be the last
	//item in the readylist 
	if ((firstid(readylist) > 0 ) && !isbadpid(firstid(readylist))){
		return TRUE;
	}
	return FALSE;
}

int is_next_process_at_the_same_priority_level(){
	/*FIXME: stub
	IMPORTANT: only call when the prerequisites for running a user process has been met.
	*/
	return 0;
}


void place_old_sys_process(pid){
	struct procent *ptold;
	ptold = &proctab[pid];

	if (ptold->prstate == PR_CURR){
		ptold->prstate = PR_READY;
		insert(pid, readylist, ptold->prprio);	
	}
}

void place_old_user_process(pid){
	struct procent *ptold;
	ptold = &proctab[pid];

	if (ptold->user_process){
		if (ptold->prstate == PR_CURR){
			ptold->prstate = PR_READY;
			//FIXME: should we insert it here NO MATTER WHAT?
			// what happens if it's at the end of quantum slice
			// what about if it's a scheduling event triggered.
		    enqueue_mlfq(pid);	
		}
	}
}

/*------------------------------------------------------------------------
 *  resched  -  Reschedule processor to highest priority eligible process
 *------------------------------------------------------------------------
 */
void	resched(void)		/* Assumes interrupts are disabled	*/
{
	struct procent *ptold;	/* Ptr to table entry for old process	*/
	struct procent *ptnew;	/* Ptr to table entry for new process	*/
	pid32  oldpid = currpid;			/* Keeping track of the old pid         */

	/* If rescheduling is deferred, record attempt and return */

	if (Defer.ndefers > 0) {
		Defer.attempt = TRUE;
		return;
	}

	/* Point to process table entry for the current (old) process */
	ptold = &proctab[currpid];
	/* system process that's running and still has better priority than
	   other system processes, we just keep that running. FIXME: might be
	   a source of error.
	*/
	//TODO: let's think about the consequences of this decision
	if ((!ptold->user_process) && (currpid > 0)  && (ptold->prstate == PR_CURR)){
		if (ptold->prprio > firstkey(readylist)) {
			return;
		}
	}

	/* what is the appropriate place for this process to go */
	if (ptold->user_process){
		//PART1: check if the initial condition for skipping ctxsw for the old process
		//has been met. 
		// 0. We are in the middle of a time slice for that priority level.
		// 1. prptr->pr_level > 0
		// 2. prptr->time_allotment > 0 (if the process used up its time allotment, we have to
		//						 		push it to a lower queue, although an exception is if it's
		//								at the lowest level, we might continue running it?)
		place_old_user_process(currpid);
	}
	else {
		place_old_sys_process(currpid);
	}
	/* pick the next process to run */	
	if (readylist_has_nonnull_sys_process()){
		currpid = dequeue(readylist);
	} else if (readylist_has_only_nullprocess()){
		if (nonempty_mlfq()){
			//PART2: if in the middle of MPQ or LPQ quantum slice, abort ctxswitch
			currpid = dequeue_mlfq();
		} else {
			currpid = dequeue(readylist);
		}
	} else if (isempty(readylist)){
		if (nonempty_mlfq()){
			//PART2: if in the middle of MPQ or LPQ quantum slice, abort ctxswitch
			currpid = dequeue_mlfq();
		} else {
			//does this ever happen?
			currpid = currpid; //don't change it?
			//old and new pid are the same, no ctxsw needed?
			return;
		}
	}

	/* new process has been picked, finish the context switch */
	ptnew = &proctab[currpid];
	ptnew->prstate = PR_CURR;
	preempt = QUANTUM;		/* Reset time slice for process	*/
	quantum_counter = 0;    /* resetting quantum_counter for lowpq & hpq */
	if (oldpid != currpid){
		ptnew->num_ctxsw += 1;
	    //DEBUG_CTXSW(oldpid, currpid);	
	}

	ctxsw(&ptold->prstkptr, &ptnew->prstkptr);

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
