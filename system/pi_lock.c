/* pi_lock.c - implements lock that avoids busy wait */
#include <xinu.h>


syscall initlock(pi_lock_t *l){
	if (pi_lock_count >= NPILOCKS){
		return SYSERR;
	}

	l->pid = -1;
	l->flag = 0;               /* available */
	l->guard = 0;
	l->waiting = newqueue();   /* initialize an empty queue */
		
	pi_lock_count++;
	return OK;
}

syscall lock(pi_lock_t *l){
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


syscall unlock(pi_lock_t *l){
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
