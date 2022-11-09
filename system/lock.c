/* lock.c - implements lock that avoids busy wait */
#include <xinu.h>

local uint32 set_park_pid = -1;
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
       	sync_printf("%d, ", ptr); 
        ptr = queuetab[ptr].qnext;
    }  
	sync_printf("%d", lastid(q));
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
		sync_printf("[%d] !!! PID %d acquired the lock\n", currpid, currpid);
		l->pid = currpid;
		l->guard = 0;
	} else {
		sync_printf("[%d] !!! PID %d TRIED to acquire the lock but FAILED.\n", currpid, currpid);
		sync_printf("[%d] flag: %d, guard: %d, lockholder: %d\n", currpid, l->flag, l->guard, l->pid);
	    //print_queue(l->waiting, "waitlist (before enq)");	
		enqueue(currpid, l->waiting);
	    //print_queue(l->waiting, "waitlist  (after enq)");	
		l->guard = 0;
		park(l->waiting); 				/* put this to sleep */	
	}

	return OK;
}


syscall unlock(lock_t *l){
	if (l->pid != currpid){
		sync_printf("[%d] UNAUTHORIZED: pid %d is trying to release the lock held by %d\n", currpid, currpid, l->pid);
		return SYSERR;
	} else {
		sync_printf("[%d] AUTHORIZED: pid %d is trying to release the lock held by %d\n", currpid, currpid, l->pid);
	}
	while(test_and_set(&l->guard, 1));
	//sync_printf("PID %d is going to unlock lock\n");


	if (isempty(l->waiting)){
		sync_printf("[%d] PID %d unlocked, no one is looking for the lock.\n", currpid, currpid);
		l->flag = 0;            /* no one is looking for the lock */
	} else {
	    //print_queue(l->waiting, "waitlist");	
		pid32 x = dq(l->waiting);
		sync_printf("[%d] lock is available, %d should get the lock.\n", currpid, x);

	    print_queue(l->waiting, "waitlist");	
	    print_queue(readylist, "readylist");	
		unpark(x); /* hold the lock for next process */

		/* FIXME TODO WARNING  THIS IS EXPERIMENTAL CODE */
		l->flag = 0; //i do not see this anywhere
	}

	l->pid = -1;
	l->guard = 0;

	return OK;
}

syscall setpark(pid32 pid){
	//indicates that the process is about to park
	// change the value to pid everytime you are about to park
	/*
	scenario 1
	==========
	pid 10 : setpark(), set_park_pid = 10
	pid 10 : park(), set_park_pid == currpid, so go ahead with it
	pid 10 : park(), reset set_park_id = -1;

	scenario 2 (when interrupted)
	==========
	pid 10 : setpark(), set_park_pid = 10
	pid 20 : setpark(), set_park_pid = 11 
			[so you want to check in set_park whether the value is different from currpid
			 if in setpark, set_park_pid != currpid, (and >=0) that means some other thread
			 is trying to set park while  
			]
	pid 10 : park(), set_park_pid == currpid, so go ahead with it
	pid 10 : park(), reset set_park_id = -1;
	

	PCB impl
	========
		lock()->setpark():
			prptr->park = 1;
		park():
			if prptr->park == 0: ctx before 	
	*/

	if (set_park_pid >= 0){
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

	//sync_printf("trying to put %d to sleep via park\n", currpid);
	struct procent *prptr;
	prptr = &proctab[currpid];
	sync_printf("[%d] putting PID %d to sleep\n", currpid, currpid);
	print_queue(readylist, "readylist before WAIT");
	print_queue(q, 		   "waitnlist before WAIT");
	prptr->prstate = PR_WAIT;
	print_queue(readylist, "readylist after  WAIT");
	print_queue(q, 		   "waitnlist after  WAIT");
	resched();

	print_queue(readylist, "readylist after rescd");
	print_queue(q, 		   "waitnlist after rescd");
	restore(mask);
	return OK;
}

syscall unpark(pid32 pid){
	/* wakes up a particular process from sleep */
	intmask 	mask;    	/* Interrupt mask		*/
	mask = disable();
	sync_printf("[%d] PID %d is going to be added to the readylist`\n", currpid, pid);
	
	ready(pid);

	restore(mask);
	return OK;
}
