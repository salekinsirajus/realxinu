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

	int ta_multiplier[3] = {1, 2, 4};
	if (!(prptr->time_allotment > 0)){
		//penalize 
		prptr->pr_level++;
		//saturate at pr_level = 2
		if (prptr->pr_level > 2){
			prptr->pr_level = 2;
		}

		//reset the time allotment based on the priority level
		prptr->time_allotment = ta_multiplier[prptr->pr_level] * TIME_ALLOTMENT;
	}

	if (prptr->pr_level == 0){
        return enqueue(pid, highpq);
	}

	if (prptr->pr_level == 1){
        return enqueue(pid, midpq);
	}

	//every other case (namely pr_level >= 2)
	return enqueue(pid, lowpq);

}

pid32 dequeue_mlfq(){
	/* pops the next in line PID from the right level */
	if (nonempty(highpq)){
	    return dequeue(highpq);
	} 
	else if (nonempty(midpq)){
		return dequeue(midpq);
	}
	else {
		return dequeue(lowpq);
	}
}

int nonempty_mlfq(){
	/* checks if any of the user level queues are empty
	 */
	return (nonempty(highpq) || nonempty(midpq) || nonempty(lowpq));
} 
