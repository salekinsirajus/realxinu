
/* lock.c - implements lock that avoids busy wait */
#include <xinu.h>


void swap(pid32 *array, int x, int y){
	pid32 tmp;
	tmp = array[x];
	array[x] = array[y];
	array[y] = tmp;
}

void bubbleSort(pid32 *array, int n) {
	bool8 swapped = 1;
	int i = 0, j;

	while (i < n - 1 && swapped) {
		swapped = 0;
		
		for (j = n - 1; j > i; j--) {
			
			if (array[j] < array[j - 1]) {
				swap(array, j, j - 1);
				swapped = 1;
			}
		}
		i++;
	}
}


syscall al_initlock(al_lock_t *l){

	if (al_lock_count >= NALOCKS){
		return SYSERR;
	}
	l->lock_id = al_lock_count;
	l->in_use = 1;

	l->pid = -1;
	l->flag = 0;               /* available */
	l->guard = 0;
	l->waiting = newqueue();   /* initialize an empty queue */
		
	lockentry *lentry;
	lentry = &locktab[l->lock_id];
	lentry->ptr = l;

	al_lock_count++;
	return OK;
}

void detect_deadlock(al_lock_t *l, pid32 pid){
	intmask mask;
 	mask = disable();		

	pid32 involved_pids[NPROC]; 
	pid32 *ptr_involved_pids;
	ptr_involved_pids = involved_pids;
	int i = 0, j;

	al_lock_t *pursued_lock;
	pid32 	   old_pursuer = -1;

	struct procent *prptr;
	lockentry    *lockptr;
	
	pid32      lockholder;  //of pursued_lock
	uint32     waiting_on;  //lock_id

	/*init loop*/
	pursued_lock = l;
	lockholder = pursued_lock->pid;

	while(lockholder != -1){
		//deadlock already formed, quit this loop
		if (i >= NPROC) break;

		if (old_pursuer == pid){
			/*found deadlock*/ 
			/* i came in increamented */
			kprintf("deadlock_detected=");

			bubbleSort(ptr_involved_pids, i); 
		    for (j=0; j<i; j++){
				if ((j > 0) && (j < i)) kprintf("-");
				kprintf("P%d", involved_pids[j]);
			}	
			kprintf("\n");

			break;
		}
		// find the pcb entry for the lockholder process
	    prptr = &proctab[lockholder];
		// find if the lokcholder is waiting on another lock
		waiting_on = prptr->waiting_on_lock;  //lock_id
		// if not, there is no chain
		if (waiting_on == -1){
			// let go
			break;
		}

		// there is some lock (waiting_on) that the lockholder wants
		// now he became the pursuer
		lockptr = &locktab[waiting_on];
		// find the lock the lockholder wants
		pursued_lock = lockptr->ptr;
	 	old_pursuer = lockholder;	
		involved_pids[i] = old_pursuer;
		i++;
		lockholder = pursued_lock->pid;

	}

	restore(mask);
	return;
}


syscall al_trylock(al_lock_t *l){
	/* 
		tries to obtain a lock and it returns immediately to the
		caller if the lock is already held. The function returns 
		true if it has obtained the lock, and false if it hasn’t.
	*/
	if (test_and_set(&l->flag, 1)){
		//failed
		return SYSERR;
	} else {
		//got the lock;
		// TODO: double check if this is working (do we need to change guard and stuff?)
		l->pid = currpid;
		return OK;	
	}

}

syscall al_lock(al_lock_t *l){
	struct procent *prptr = &proctab[currpid];
	prptr->waiting_on_lock = l->lock_id;
	detect_deadlock(l, currpid);

	while(test_and_set(&l->guard, 1) == 1);
	if (l->flag == 0){
		l->flag = 1;
		prptr->waiting_on_lock = -1;  //Got the lock, does not want it anymore
		l->pid = currpid;
		l->guard = 0;
	} else {
		enqueue(currpid, l->waiting);
		
		l->guard = 0;
		park(l->waiting); 				/* put this to sleep */	
	}

	return OK;
}


syscall al_unlock(al_lock_t *l){

	if (l->pid != currpid){
		return SYSERR;
	}
	
	while(test_and_set(&l->guard, 1) == 1);

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
