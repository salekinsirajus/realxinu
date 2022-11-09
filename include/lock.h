/* lock.h - spinlock */
#ifndef NSPINLOCKS
#define NSPINLOCKS 	20   /* maximum number of spinlocks */
#endif

#ifndef NLOCKS
#define NLOCKS 	20       /* maximum number of locks     */
#endif

typedef struct sl_lock_t{
	int   lock_id;
	pid32     pid;
    uint32   flag;
} sl_lock_t;

typedef struct lock_t{
	uint32   flag;		/* whether the lock is taken         */
	uint32  guard;		/* guard before acquiring the lock   */
	pid32     pid;		/* which process is holding the lock */
    qid16 waiting;		/* processes waiting for the lock    */
} lock_t;

extern uint32 sl_lock_count;
extern uint32 lock_count;
