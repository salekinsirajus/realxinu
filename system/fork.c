/* fork.c - implements a unix-like fork system call */

#include <xinu.h>

/*
 * Waht does fork() do?
 * In unix, it creates a new child from the parent, copies the data over. (what kind of data?)
 * and then continues executing
 * copy over parents everything. Except for stack.
 * function f1()
 * */
pid32 fork(){

    // The CALL fork function pushed the EIP onto the stack
    // Let's get it from there
    // FIXME: are these unsigned long or uint32
    
    unsigned long *sp, *fp, *ebx, *esi, *edi;

    asm("movl %%esp, %0\n" :"=r"(sp));
    asm("movl %%ebp, %0\n" :"=r"(fp));
    asm("movl -4(%%ebp), %0\n" :"=r"(ebx)); 
    asm("movl -8(%%ebp), %0\n" :"=r"(esi)); 
    asm("movl -12(%%ebp), %0\n" :"=r"(edi)); 

    uint32      savsp, *pushsp;

    uint32      ssize;              /* Stack size in bytes      */
    intmask     mask;               /* Interrupt mask       */
    pid32       pid;                /* Stores new process id    */
    struct  procent *prptr;  /* parent process entry */
    struct  procent *parent_prptr;  /* parent process entry */

    int32             i;
    uint32       *saddr;            /* Stack address        */

    mask = disable();
    parent_prptr = &proctab[(pid32)getpid()];

    ssize = parent_prptr->prstklen;

    if (((pid=newpid()) == SYSERR) ||
        ((saddr = (uint32 *)getstk(ssize)) == (uint32 *)SYSERR)) {
        restore(mask);
        return SYSERR;
    }

    prptr = &proctab[pid];
    prcount++;

    /* Initialize process table entry for new process */
    prptr->prstate = PR_SUSP;   /* Initial state is suspended   */
    prptr->prprio = parent_prptr->prprio;
    prptr->prstkbase = (char *)saddr;
    prptr->prstklen = ssize;
    prptr->prname[PNMLEN-1] = NULLCH;
    for (i=0 ; i<PNMLEN-1 && (prptr->prname[i]=parent_prptr->prname[i])!=NULLCH; i++)
        ;
    prptr->prsem = -1;
    prptr->prparent = (pid32)getpid();
    prptr->prhasmsg = FALSE;

    /* Set up stdin, stdout, and stderr descriptors for the shell   */
    prptr->prdesc[0] = CONSOLE;
    prptr->prdesc[1] = CONSOLE;
    prptr->prdesc[2] = CONSOLE;

    /* copy stuff from parents stack up until the frame pointer */
    unsigned long* it = (unsigned long)parent_prptr->prstkbase;

    while(it>=(fp)){
        *saddr=*it;
        it--;
        saddr--;
    }
    it++;
    saddr++;

    /* since we copied the parent stack verbatim, we will have to update the
     * memory addresses in the child stack so it points to the correct memory
     * locations. */    
    unsigned long *mem_loc = saddr;
    unsigned long diff;
 
    diff = (unsigned long)parent_prptr->prstkbase - (unsigned long)prptr->prstkbase;

    while(mem_loc < (unsigned long)prptr->prstkbase){
       *mem_loc = *mem_loc - ((unsigned long)diff);
       mem_loc = *mem_loc;
    }

    *--saddr = 0x00000200;   /* New process runs with interrupt enabled*/

    /* Basically, the following emulates an x86 "pushal"instruction*/
    
    *--saddr = NPROC;       /* %eax; */
    *--saddr = 0;           /* %ecx */
    *--saddr = 0;           /* %edx */
    *--saddr = ebx;         /* %ebx */
    
    *--saddr = 0;           /* %esp; value filled in below */
    pushsp = saddr;         /* Remember this location   */
    *--saddr = fp;          /* %ebp (should contain the previous frame's fp */
    *--saddr = esi;         /* %esi */
    *--saddr = edi;         /* %edi */
    *pushsp = (unsigned long) (prptr->prstkptr = (char *)saddr);
    restore(mask);

    prptr->prstate = PR_READY;
    //TODO: should I call it from here?
    insert(pid,readylist,prptr->prprio);
      
    return pid;
}
