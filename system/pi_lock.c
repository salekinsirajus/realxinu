/* pi_lock.c - implements lock that avoids busy wait */
#include <xinu.h>
void reset_priority(pid32 pid);
void inherit_priority(pid32 lock_holder, pid32 wants_lock);

syscall pi_initlock(pi_lock_t *l){
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

void reset_priority(pid32 pid){
	/* reset the priority to the initial level*/
	if (isbadpid(pid)){
		return;
	}

	/* disable interrupt */
	intmask mask;
	mask = disable();

	struct procent *prptr;
	prptr = &proctab[pid];

	// changes from prprio TO init_prprio
	if (prptr->prprio != prptr->init_prprio){
		kprintf("priority_change=P%d::%d-%d\n", pid, prptr->prprio, prptr->init_prprio);
		prptr->prprio = prptr->init_prprio;
		resched();
	}

	/* enable interrupt*/
	restore(mask);
	return;
}

void inherit_priority(pid32 lock_holder, pid32 wants_lock){
	if (isbadpid(lock_holder) || isbadpid(wants_lock)){
		return;
	}

	/* disable interrupt */
	intmask mask;
	mask = disable();

	struct procent *holder, *waiter;
	holder = &proctab[lock_holder];
	waiter = &proctab[wants_lock];

	// goes from waiter TO holder
	if (holder->prprio < waiter->prprio){
		/* inherit the waiting processes priority */
		kprintf("priority_change=P%d::%d-%d\n", lock_holder, holder->prprio, waiter->prprio);
		holder->prprio = waiter->prprio;
		resched();
	}
	
	/* enable interrupt*/
	restore(mask);
	return;
}

syscall pi_lock(pi_lock_t *l){
	while(test_and_set(&l->guard, 1)){
		sleep(QUANTUM);
	}
	if (l->flag == 0){
		l->flag = 1;
		l->pid = currpid;
		l->guard = 0;
	} else {
		//synce_printf("about to call inherit priority\n");
		inherit_priority(l->pid, currpid);
		struct procent *prptr;
		prptr = &proctab[currpid];
		enqueue(currpid, l->waiting);
		l->guard = 0;
		park(l->waiting); 				/* put this to sleep */	
	}

	return OK;
}


syscall pi_unlock(pi_lock_t *l){
	if (l->pid != currpid){
		return SYSERR;
	}

	while(test_and_set(&l->guard, 1)){
		sleep(QUANTUM);
	}
	reset_priority(currpid);
	
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
