
/* lock.c - implements lock that avoids busy wait */
#include <xinu.h>


syscall al_initlock(al_lock_t *l){

	if (al_lock_count >= NALOCKS){
		return SYSERR;
	}
	l->lock_id = al_lock_count;
	l->in_use = 1;

	/* keep a reference in the lock linked list */
	lock_node *p;
	p = &lll;
	int spincount = 0;
	while (p->lock_id != l->lock_id){
		kprintf("spinning here\n");
		spincount++;
		p = p->prev;
		if (spincount > 20) break;
	}
	p->ptr = l;

	kprintf("[%d]: original lock location: %X, lock_id:%d\n", currpid, &l, l->lock_id);
//	l = x; //&allocktab[i];
	kprintf("[%d]: after assignment lock location: %X, lock_id:%d\n", currpid, &l, l->lock_id);

	kprintf("[%d] pid %d is initializing lock %d, al_lock_count %d.\n", currpid, currpid, l->lock_id, al_lock_count);
	l->pid = -1;
	l->flag = 0;               /* available */
	l->guard = 0;
	l->waiting = newqueue();   /* initialize an empty queue */
		
	al_lock_count++;
	return OK;
}

void detect_deadlock(al_lock_t *l, pid32 pid){
	intmask mask;
 	mask = disable();		
	// the lock I want is held by someone else
	
	if (isbadpid(l->pid)){
		kprintf("[%d] not a legit pid %d, the lock is up for grab\n", currpid, l->pid);
		restore(mask);
		return;
	}	

	if (proctab[l->pid].prstate == PR_WAIT){
		kprintf("[%d] potential deadlock, %d is waiting on lock %d.\n", currpid, l->pid, l->lock_id);
		kprintf("[%d] lockholder %d is waiting on lock %d\n", currpid, l->lock_id, queuetab[l->pid].qkey);	
		restore(mask);
		return;
	}

	restore(mask);
}

bool8 al_trylock(al_lock_t *l){
	/* 
		tries to obtain a lock and it returns immediately to the
		caller if the lock is already held. The function returns 
		true if it has obtained the lock, and false if it hasn’t.
	*/
	if (test_and_set(l->flag, 1)){
		//failed
		return 0;
	} else {
		//got the lock;
		// TODO: double check if this is working (do we need to change guard and stuff?)
		l->pid = currpid;
		return 1;	
	}

}

syscall al_lock(al_lock_t *l){
	//since we are returning from detect deadlock code it should be fine although
	//we are only checking whether the deadlock is held a pid not the flag or guard
	//variable

	/* finding the actual lock */

//	l = &allocktab[l->lock_id];

	detect_deadlock(l, currpid);

	while(test_and_set(&l->guard, 1) == 1);
	sync_printf("[%d] lock_id: %d, flag: %d, guard: %d, lockholder: %d\n", currpid, l->lock_id, l->flag, l->guard, l->pid);
	if (l->flag == 0){
		l->flag = 1;
		sync_printf("[%d] !!! PID %d acquired the [lock %d]\n", currpid, currpid, l->lock_id);
		l->pid = currpid;
		sync_printf("[%d] vars after acquisition == lock_id: %d, flag: %d, guard: %d, lockholder: %d\n", currpid, l->lock_id, l->flag, l->guard, l->pid);
		l->guard = 0;
		sync_printf("[%d] vars after guard release == lock_id: %d, flag: %d, guard: %d, lockholder: %d\n", currpid, l->lock_id, l->flag, l->guard, l->pid);
	} else {
		sync_printf("[%d] !!! PID %d TRIED to acquire [lock %d] but FAILED.\n", currpid, currpid, l->lock_id);
		sync_printf("[%d] lock_id: %d, flag: %d, guard: %d, lockholder: %d\n", currpid, l->lock_id, l->flag, l->guard, l->pid);
		enqueue(currpid, l->waiting);
		queuetab[currpid].qkey = l->lock_id;
		
		l->guard = 0;
		park(l->waiting); 				/* put this to sleep */	
	}

	return OK;
}


syscall al_unlock(al_lock_t *l){
//	l = &allocktab[l->lock_id];

	if (l->pid != currpid){
		sync_printf("[%d] UNAUTHORIZED: pid %d is trying to release the lock held by %d\n", currpid, currpid, l->pid);
		return SYSERR;
	} else {
		sync_printf("[%d] AUTHORIZED: pid %d is trying to release the lock held by %d\n", currpid, currpid, l->pid);
	}
	while(test_and_set(&l->guard, 1) == 1);
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
