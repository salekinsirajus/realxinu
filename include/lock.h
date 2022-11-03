/* lock.h - spinlock */
#ifndef NSPINLOCKS
#define NSPINLOCKS 	20   /* maximum number of spinlocks */
#endif

//typedef uint32 sl_lock_t;
typedef struct sl_lock_t{
	int lock_id;
	pid32  pid;
    uint32 flag;
} sl_lock_t;
extern uint32 sl_lock_count;
