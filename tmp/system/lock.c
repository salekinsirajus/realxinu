/* lock.c - implements lock that avoids busy wait */
#include <xinu.h>

local uint32 set_park_flag = -1;
syscall park(qid16 q);
syscall unpark(pid32 pid);

pid32 dq(qid16 q){
	/* removes the first item from the q */
    if (isempty(q)){
        return SYSERR;
    }

    pid32 pid = firstid(q);
	pid32 prev, next;

	next = queuetab[pid].qnext;	/* Following node in list	*/
	prev = queuetab[pid].qprev;	/* Previous node in list	*/

	queuetab[queuehead(q)].qnext = next;
	queuetab[next].qprev = queuehead(q);
	
	queuetab[pid].qnext = EMPTY;
	queuetab[pid].qprev = EMPTY;

	return pid;
    
}

void print_queue(qid16 q, char *name){
	return; //disabled
	sync_printf("[%d]", currpid);
    qid16 ptr = firstid(q);
	struct procent *prptr;
	if (isempty(q)){
		sync_printf("%s: []\n", name);
		return;
	}

	sync_printf("%s: [", name);
    while (ptr != lastid(q)){
		prptr = &proctab[ptr];
       	sync_printf("%d(%d), ", ptr, prptr->prprio); 
        ptr = queuetab[ptr].qnext;
    }  
	sync_printf("%d(%d)", lastid(q), prptr->prprio);
	sync_printf("]\n");
}

syscall initlock(lock_t *l){
	if (lock_count >= NLOCKS){
		return SYSERR;
	}

	l->pid = -1;
	l->flag = 0;               /* available */
	l->guard = 0;
	l->waiting = newqueue();   /* initialize an empty queue */
		
	lock_count++;
	return OK;
}

syscall lock(lock_t *l){
	while(test_and_set(&l->guard, 1));
	if (l->flag == 0){
		l->flag = 1;

		l->pid = currpid;
		l->guard = 0;
	} else {
		enqueue(currpid, l->waiting);
		l->guard = 0;
		park(l->waiting); 				/* put this to sleep */	
	}

	return OK;
}


syscall unlock(lock_t *l){
	if (l->pid != currpid){
		return SYSERR;
	}

	while(test_and_set(&l->guard, 1));


	if (isempty(l->waiting)){
		l->flag = 0;            /* no one is looking for the lock */
	} else {
		pid32 x = dq(l->waiting);

		unpark(x); /* hold the lock for next process */

		/* FIXME TODO WARNING  THIS IS EXPERIMENTAL CODE */
		l->flag = 0; //i do not see this anywhere
	}

	l->pid = -1;
	l->guard = 0;

	return OK;
}

syscall setpark(pid32 pid){
	//indicates that a process is about to park
       if (set_park_flag >= 0){
               return;
       }

}

syscall in_queue(qid16 q, pid32 pid){
	/*checks if a process with pid in the qeueue given*/
	if (isempty(q)) return 0;
	pid32 ptr = firstid(q);

	while (ptr != lastid(q)){
		if (ptr == pid) return 1;
		ptr = queuetab[ptr].qnext;
	}
	if (pid == lastid(q)) return 1;

	return 0;
}

syscall park(qid16 q){
	/*puts the caller process to sleep */
	intmask 	mask;    	/* Interrupt mask		*/
	mask = disable();

	struct procent *prptr;
	prptr = &proctab[currpid];
	prptr->prstate = PR_WAIT;
	resched();


	restore(mask);
	return OK;
}

syscall unpark(pid32 pid){
	/* wakes up a particular process from sleep */
	intmask 	mask;    	/* Interrupt mask		*/
	mask = disable();
	ready(pid);

	restore(mask);
	return OK;
}
