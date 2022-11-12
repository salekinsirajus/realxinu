
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

	sync_printf("[%d]: original lock location: %X, lock_id:%d\n", currpid, &l, l->lock_id);

	sync_printf("[%d]: after assignment lock location: %X, lock_id:%d\n", currpid, &l, l->lock_id);

	sync_printf("[%d] pid %d is initializing lock %d, al_lock_count %d.\n", currpid, currpid, l->lock_id, al_lock_count);
	l->pid = -1;
	l->flag = 0;               /* available */
	l->guard = 0;
	l->waiting = newqueue();   /* initialize an empty queue */
		
	lockentry *lentry;
	lentry = &locktab[l->lock_id];
	lentry->ptr = l;

	sync_printf("in the table, lock_id: %d, on the lock_object: %d\n", lentry->lock_id, lentry->ptr->lock_id);
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
			sync_printf("[%d] lockholder not waiting on any lock\n", lockholder);
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


bool8 al_trylock(al_lock_t *l){
	/* 
		tries to obtain a lock and it returns immediately to the
		caller if the lock is already held. The function returns 
		true if it has obtained the lock, and false if it hasn’t.
	*/
	if (test_and_set(&l->flag, 1)){
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
	struct procent *prptr = &proctab[currpid];
	prptr->waiting_on_lock = l->lock_id;
	sync_printf("[%d] PID %d wants the lock %d, marked at PCB: %d\n", currpid, currpid, l->lock_id, prptr->waiting_on_lock);
	detect_deadlock(l, currpid);

	while(test_and_set(&l->guard, 1) == 1);
	sync_printf("[%d] lock_id: %d, flag: %d, guard: %d, lockholder: %d\n", currpid, l->lock_id, l->flag, l->guard, l->pid);
	if (l->flag == 0){
		l->flag = 1;
		prptr->waiting_on_lock = -1;  //Got the lock, does not want it anymore
		sync_printf("[%d] PID %d GOT the lock %d, marked at PCB: %d\n", currpid, currpid, l->lock_id, prptr->waiting_on_lock);
		sync_printf("[%d] !!! PID %d acquired the [lock %d]\n", currpid, currpid, l->lock_id);
		l->pid = currpid;
		sync_printf("[%d] vars after acquisition == lock_id: %d, flag: %d, guard: %d, lockholder: %d\n", currpid, l->lock_id, l->flag, l->guard, l->pid);
		l->guard = 0;
		sync_printf("[%d] vars after guard release == lock_id: %d, flag: %d, guard: %d, lockholder: %d\n", currpid, l->lock_id, l->flag, l->guard, l->pid);
	} else {
		sync_printf("[%d] !!! PID %d TRIED to acquire [lock %d] but FAILED.\n", currpid, currpid, l->lock_id);
		sync_printf("[%d] lock_id: %d, flag: %d, guard: %d, lockholder: %d\n", currpid, l->lock_id, l->flag, l->guard, l->pid);
		enqueue(currpid, l->waiting);
		
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
		//struct procent *prptr = &proctab[x];
		//prptr->waiting_on_lock = -1; //unmark which lock he is waiting on
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
