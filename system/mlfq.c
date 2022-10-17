#include <xinu.h>

pid32 enqueue_mlfq(pid32 pid){
	/* Adds a user process to the correct priority level */
	struct procent *prptr;
	prptr = &proctab[pid];	
	
	/* First time added gets to the highest priority level */
	if (prptr->runtime == 0){
		return enqueue(pid, highpq);
	} else {
        return enqueue(pid, highpq); //for now
    }
}

pid32 dequeue_mlfq(){
	//FIXME: this is a STUB
	/* pops the next in line PID from the right level */
	return dequeue(highpq);
}

int nonempty_mlfq(){
	/* FIXME: this is a stub
	 * checks if any of the user level queues are empty
	 */
	return nonempty(highpq);
} 
