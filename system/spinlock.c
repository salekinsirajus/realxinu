#include <xinu.h>

#define FREE   0
#define LOCKED 1

syscall sl_initlock(sl_lock_t *l){
	if (sl_lock_count >= NSPINLOCKS){
		return SYSERR;
	}

	l->flag = FREE;
	l->lock_id = sl_lock_count;
	l->pid = -1;
	sl_lock_count++;

	return OK;
}

syscall sl_lock(sl_lock_t *l){
	while(test_and_set(l->flag, LOCKED) == FREE){
		l->pid = currpid;
		return OK;  //worked
	}

	return 1;      //did not work?
}

syscall sl_unlock(sl_lock_t *l){
    /* FIXME: how would we know if this lock is owned by this process? */
    
    if (l->pid == currpid){
    	l->flag = FREE;
		l->pid = -1;
    	return OK;
    }

	//FIXME: should return syserr when it's unauthorized access
    return SYSERR;
}
