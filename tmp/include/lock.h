/* lock.h - spinlock */
#ifndef NSPINLOCKS
#define NSPINLOCKS 	20   /* maximum number of spinlocks */
#endif

#ifndef NLOCKS
#define NLOCKS 	20       /* maximum number of locks     */
#endif

#ifndef NALOCKS
#define NALOCKS 	20       /* maximum number of locks     */
#endif

#ifndef NPILOCKS
#define NPILOCKS 	20       /* maximum number of locks     */
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

typedef struct al_lock_t{
	uint32   flag;		/* the "lock" variable (taken or not)*/
	uint32  guard;		/* guard before acquiring the lock   */
	pid32     pid;		/* which process holds the lock atm  */
    qid16 waiting;		/* queue of PIDs waiting for the lock*/
	uint32 lock_id;		/* use as a primary key for accounting */
	bool8  in_use;		/* whether this lock is initialized  */
} al_lock_t;

typedef struct lockentry{
	uint32  lock_id;
	al_lock_t  *ptr;
} lockentry;

extern lockentry locktab[];

typedef struct pi_lock_t{ /* priority inversion              */
	uint32   flag;		  /* whether the lock is taken         */
	uint32  guard;	      /* guard before acquiring the lock   */
	pid32     pid;        /* which process is holding the lock */
    qid16 waiting;		  /* processes waiting for the lock    */
	uint32 lock_id;
} pi_lock_t;

extern uint32 sl_lock_count;
extern uint32 lock_count;
extern uint32 al_lock_count;
extern uint32 pi_lock_count;
