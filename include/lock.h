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
	uint32   flag;		/* whether the lock is taken         */
	uint32  guard;		/* guard before acquiring the lock   */
	pid32     pid;		/* which process is holding the lock */
    qid16 waiting;		/* processes waiting for the lock    */
	uint32 lock_id;
	bool8  in_use;
} al_lock_t;

typedef struct pi_lock_t{ /* priority inversion              */
	uint32   flag;		  /* whether the lock is taken         */
	uint32  guard;	      /* guard before acquiring the lock   */
	pid32     pid;        /* which process is holding the lock */
    qid16 waiting;		  /* processes waiting for the lock    */
} pi_lock_t;

extern uint32 sl_lock_count;
extern uint32 lock_count;
extern uint32 al_lock_count;
extern uint32 pi_lock_count;

typedef struct lock_node{
	uint32 		lock_id;
	al_lock_t 	*ptr;
	struct lock_node*  prev;
} lock_node;

extern lock_node lll;

