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

int get_next_pr_level(){
	if (nonempty(highpq)) return 0;
	if (nonempty(midpq)) return 1;
	if (nonempty(lowpq)) return 2;
}

pid32 get_from_the_end_of_q(int q_level){
	if (q_level == 1){
		return getlast(midpq);
	}
	if (q_level == 2){
		return getlast(lowpq);
	}
}

int should_user_pr_continue_wo_ctxsw(pid32 pid){
	/*FIXME: stub
	IMPORTANT: only call when the prerequisites for running a user process has been met.
	*/
	struct procent *prptr;
	uint32 next_pr_level = get_next_pr_level(); 
	int ta_multiplier[3] = {1,2,4};

	prptr = &proctab[pid];

//	sync_printf("next_pr_level: %d\n", next_pr_level);
//	sync_printf("process pr level: %d\n", prptr->pr_level);
	//does not apply for highpq level
	if ((prptr->pr_level == 0) || (!prptr->user_process) || (next_pr_level == 0)) return FALSE; 	
	//if the process used up the time allotment then no
	int cond1 = (prptr->time_allotment > 0);
//	sync_printf("time allotment: %d\n", prptr->time_allotment);
	if (!cond1) return FALSE;
	//there is no new process that came at a higher level
	//or if this process has just got deprioritized we won't let it run
	if (next_pr_level != prptr->pr_level) return FALSE;

    int in_the_middle_of_timeslice = (quantum_counter % ta_multiplier[prptr->pr_level]);	
//	sync_printf("qc = %d, in the middle? %d, ta_multiplier: %d\n",
//		quantum_counter, in_the_middle_of_timeslice, ta_multiplier[prptr->pr_level]);
	
	if (in_the_middle_of_timeslice == 0){

		return FALSE;
	} 

	//Everything looks good, now getting the correct location
	if (next_pr_level == 1){
		if (lastid(midpq) != pid){
			return FALSE;
		}
	}	

	if (next_pr_level == 2){
		if (lastid(lowpq) != pid){
			return FALSE;
		}
	}	

	return TRUE;
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

	int resched_evoked_by_clock_handler = (ptold->prstate == PR_CURR);
	/* FIXME: watch out for this
	sync_printf("preempt==qunatum? %d\n", preempt == QUANTUM);
	sync_printf("prstate==PR_CURR? %d\n", ptold->prstate == PR_CURR);
	*/
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
		if (nonempty_mlfq()){
/*
				currpid = dequeue_mlfq();
*/
			if (should_user_pr_continue_wo_ctxsw(oldpid)){
				currpid = get_from_the_end_of_q(get_next_pr_level());	
			} else {
				currpid = dequeue_mlfq();
			}
		} else {
			currpid = dequeue(readylist);
		}
	} else if (isempty(readylist)){
		if (nonempty_mlfq()){
/*
				currpid = dequeue_mlfq();
*/
			if (should_user_pr_continue_wo_ctxsw(oldpid)){
				currpid = get_from_the_end_of_q(get_next_pr_level());	
			} else {
				currpid = dequeue_mlfq();
			}
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
	if (!resched_evoked_by_clock_handler){
		quantum_counter = 0;    /* Reset the quantum_counter for everybody */
	}
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
