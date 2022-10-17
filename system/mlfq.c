#include <xinu.h>

pid32 enqueue_mlfq(pid32 pid){
	/* Adds a user process to the correct priority level */
	struct procent *prptr;
	prptr = &proctab[pid];	
	
	/* First time added gets to the highest priority level */
	if (prptr->runtime == 0){
		return enqueue(pid, highpq);
	} 

	/* Check the priority level for this process. Also check the
	   Allotment usage. If used up all the allotted time, it gets
	   one level down. */

	if (!(prptr->time_allotment > 0)){
		//penalize 
		prptr->pr_level++;
	}

	if (prptr->pr_level == 0){
        return enqueue(pid, highpq); //for now
	}
	if (prptr->pr_level >= 1){
		return enqueue(pid, lowpq);
	}
}

pid32 dequeue_mlfq(){
	//FIXME: this is a STUB
	/* pops the next in line PID from the right level */
	if (nonempty(highpq)){
	    return dequeue(highpq);
	}
	else {
		return dequeue(lowpq);
	}
}

int nonempty_mlfq(){
	/* FIXME: this is a stub
	 * checks if any of the user level queues are empty
	 */
	return (nonempty(highpq) || nonempty(lowpq));
} 
