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
			// we do not use insert cause it requires a key so we using enqueue instead
			//enqueue(pid, highpq);
			// we can try using insert using pid as a key
			insert(pid, highpq, pid);
		}
	}
}

pid32 dequeue_lotteryq(){

	qid16 ptr = firstid(highpq);
	struct procent *prptr;

	uint32 ticket_sum = 0;
	uint32 eligible_process_count = 0;
	while (ptr != queuetail(highpq)){
		prptr = &proctab[ptr];
		if (prptr->tickets > 0){
			eligible_process_count++;
			ticket_sum += prptr->tickets;
		}
		ptr = queuetab[ptr].qnext;
	}

	// no point in holding lottery if there is only one process
	//if (eligible_process_count == 1) return dequeue(highpq);

	/* should not happen, since this function would not be called */
	if (ticket_sum == 0){
		sync_printf("returns from ticket_sum check: %d\n", ticket_sum);
		return SYSERR;
	}

	/* hold lottery */
	uint32 winner = rand() % ticket_sum;
	sync_printf("held the lottery, winner is %d\n", winner);

	/* find the winner */
	ptr = firstid(highpq);
	ticket_sum = 0;
	while (ptr != queuetail(highpq)){
		prptr = &proctab[ptr];
		if (prptr->tickets > 0){
			ticket_sum += prptr->tickets;
			sync_printf("checking ticket_sum %d against winnner %d\n", ticket_sum, winner);
			if (ticket_sum > winner){
				//found winner
				sync_printf("returning winner process: %d\n", ptr);
				return getitem(ptr);
			}
		}
		ptr = queuetab[ptr].qnext;
	}

	//Should never come here
	return SYSERR;
}

int nonempty_lotteryq(){
	//return nonempty(highpq); //testing FIXME:
	if nonempty(highpq){
		//check if there is at least one process with tickets > 0

		qid16 ptr = firstid(highpq);
		struct procent *prptr;

		while (ptr != queuetail(highpq)){
			prptr = &proctab[ptr];
			if (prptr->tickets > 0) return TRUE;
			ptr = queuetab[ptr].qnext;
		}

		return FALSE;
	}

	return FALSE;
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
	uint32 quantum_multiplier = 1;      /* based on different levels */

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
		place_old_user_process(currpid);
	}
	else {
		place_old_sys_process(currpid);
	}
	/* pick the next process to run */
	if (readylist_has_nonnull_sys_process()){
		currpid = dequeue(readylist);
	} else if (readylist_has_only_nullprocess()){
		if (nonempty_lotteryq()){
			currpid = dequeue_lotteryq();
		} else {
			currpid = dequeue(readylist);
		}
	} else if (isempty(readylist)){
		if (nonempty_lotteryq()){
			currpid = dequeue_lotteryq();
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
	preempt = QUANTUM * quantum_multiplier;		/* Reset time slice for process	*/
	if (oldpid != currpid){
		ptnew->num_ctxsw += 1;
	    DEBUG_CTXSW(oldpid, currpid);
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
