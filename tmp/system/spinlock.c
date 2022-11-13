#include <xinu.h>

#define FREE   0
#define LOCKED 1

syscall sl_initlock(sl_lock_t *l){
	if (sl_lock_count > NSPINLOCKS){
		return SYSERR;
	}

	l->flag = FREE;
	l->lock_id = sl_lock_count;
	//l->pid = -1;  //when a lock is reinitialized the PID gets messed up?
	sl_lock_count++;

	//sync_printf("Lock %d: PID %d initialized lock %d\n",l->lock_id, currpid, l->lock_id);
	return OK;
}

syscall sl_lock(sl_lock_t *l){
	while(test_and_set(&l->flag, LOCKED));
	l->pid = currpid;
	//sync_printf("Lock %d: PID %d got a hold of lock %d. PID in lock: %d\n", l->lock_id, currpid, l->lock_id, l->pid);
	return OK;  //worked
}

syscall sl_unlock(sl_lock_t *l){
    /* FIXME: how would we know if this lock is owned by this process? */
    
	//sync_printf("Lock %d: PID %d trying to release lock %d, PID in lock: %d\n", l->lock_id, currpid, l->lock_id, l->pid);
    if (l->pid == currpid){
		l->pid = -1;
    	l->flag = FREE;
	//	sync_printf("Lock %d: PID %d released the lock %d!\n",l->lock_id, currpid, l->lock_id);
    	return OK;
    }

	//FIXME: should return syserr when it's unauthorized access
    return SYSERR;
}
