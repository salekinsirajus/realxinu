/*  main_pi.c  - main */

#include <xinu.h>
#include <stdlib.h>


syscall sync_printf(char *fmt, ...)
{
		return;
        intmask mask = disable();
        void *arg = __builtin_apply_args();
        __builtin_apply((void*)kprintf, arg, 100);
        restore(mask);
        return OK;
}

uint32 get_timestamp(){
    return ctr1000;
}

void run_for_ms(uint32 time){
    uint32 start = proctab[currpid].runtime;
    while (proctab[currpid].runtime-start < time);
}

void pr1(al_lock_t *mutex1, al_lock_t *mutex2, al_lock_t *mutex3){
	al_lock(mutex1);
	run_for_ms(500);
	al_lock(mutex3);
	run_for_ms(500);
	al_unlock(mutex3);
	al_unlock(mutex1);
    return;
}

void pr1_with_trylock(al_lock_t *mutex1, al_lock_t *mutex2, al_lock_t *mutex3){
	al_lock(mutex1);
	run_for_ms(500);
	uint32 tries = 10;
	while(tries){
		if (al_trylock(mutex3) == SYSERR){
			sleepms(1000);
			tries--;
		}
	}
	run_for_ms(500);
	al_unlock(mutex3);
	al_unlock(mutex1);
    return;
}

void pr2(al_lock_t *mutex1, al_lock_t *mutex2, al_lock_t *mutex3){
	al_lock(mutex2);
	run_for_ms(500);
	al_lock(mutex1);
	run_for_ms(500);
	al_unlock(mutex1);
	al_unlock(mutex2);
    return;
}

void pr2_with_trylock(al_lock_t *mutex1, al_lock_t *mutex2, al_lock_t *mutex3){
	al_lock(mutex2);
	run_for_ms(500);
	uint32 tries = 10;
	while(tries){
		if (al_trylock(mutex1) == SYSERR){
			sleepms(1000);
			tries--;
		}
	}
	run_for_ms(500);
	al_unlock(mutex1);
	al_unlock(mutex2);
    return;
}

void pr3(al_lock_t *mutex1, al_lock_t *mutex2, al_lock_t *mutex3){
	al_lock(mutex3);
	run_for_ms(500);
	al_lock(mutex2);
	run_for_ms(500);
	al_unlock(mutex2);
	al_unlock(mutex3);
    return;
}


void pr3_with_trylock(al_lock_t *mutex1, al_lock_t *mutex2, al_lock_t *mutex3){
	al_lock(mutex3);
	run_for_ms(500);
	uint32 tries = 10;
	while(tries){
		if (al_trylock(mutex2) == SYSERR){
			sleepms(1000);
			tries--;
		}
	}
	run_for_ms(500);
	al_unlock(mutex2);
	al_unlock(mutex3);
    return;
}

process main(void){
    
	al_lock_t mutex1;
	al_lock_t mutex2;
	al_lock_t mutex3;
	al_initlock(&mutex1);
	al_initlock(&mutex2);
	al_initlock(&mutex3);

	kprintf("================= TEST1 Active Lock w/o Deadlock Prevention================\n");
    pid32 pid1 = create((void *)pr1, INITSTK, 10,"pr1", 3, &mutex1, &mutex2, &mutex3);
    pid32 pid2 = create((void *)pr2, INITSTK, 10,"pr2", 3, &mutex1, &mutex2, &mutex3);
    pid32 pid3 = create((void *)pr3, INITSTK, 10,"pr3", 3, &mutex1, &mutex2, &mutex3);

    resume(pid1);
    resume(pid2);
    resume(pid3);

	sleepms(10000); //sleeping so they can execute
	struct procent *prptr1, *prptr2, *prptr3;
	prptr1 = &proctab[pid1];
	prptr2 = &proctab[pid2];
	prptr3 = &proctab[pid3];

	kprintf("states of processes: %d, %d, %d\n", prptr1->prstate, prptr2->prstate, prptr3->prstate);
	if ((prptr1->prstate == PR_WAIT) 
	&& (prptr2->prstate == PR_WAIT) 
	&& (prptr3->prstate == PR_WAIT) 
	){
		kprintf("All proceses are in PR_WAIT(%d) state.\n", PR_WAIT);	
    	kprintf("TEST PASSED!\n");
		kill(pid3);
		receive();
		kill(pid2);
		receive();
		kill(pid1);
		receive();
	}

	kprintf("=================================TEST1 DONE================================\n");


	kprintf("================= TEST1 Active Lock WITH Deadlock Prevention================\n");
    pid1 = create((void *)pr1_with_trylock, INITSTK, 10,"pr1", 3, &mutex1, &mutex2, &mutex3);
    pid2 = create((void *)pr2_with_trylock, INITSTK, 10,"pr2", 3, &mutex1, &mutex2, &mutex3);
    pid3 = create((void *)pr3_with_trylock, INITSTK, 10,"pr3", 3, &mutex1, &mutex2, &mutex3);

    resume(pid1);
    resume(pid2);
    resume(pid3);

	prptr1 = &proctab[pid1];
	prptr2 = &proctab[pid2];
	prptr3 = &proctab[pid3];

	kprintf("states of processes: %d, %d, %d\n", prptr1->prstate, prptr2->prstate, prptr3->prstate);

	receive();
	receive();
	receive();
	
    kprintf("TEST PASSED!\n");

	kprintf("=================================TEST3 DONE================================\n");

    return OK;
}
