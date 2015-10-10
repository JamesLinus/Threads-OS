#ifndef THREADS_SYNCH_H
#define THREADS_SYNCH_H

#include <list.h>
#include <stdbool.h>

/* A counting semaphore. */
struct semaphore 
  {
    unsigned value;             /* Current value. */
    struct list waiters;        /* List of waiting threads. */
  };

void sema_init (struct semaphore *, unsigned value);
void sema_down (struct semaphore *);
bool sema_try_down (struct semaphore *);
void sema_up (struct semaphore *);
void sema_self_test (void);

/* Compare wakeup_tick of two semaphores */
bool
wakeup_tick_less(const struct list_elem *a,
                 const struct list_elem *b,
                 void *aux);

/* Compare priorities of two threads */
bool
higher_priority(const struct list_elem *a,
                const struct list_elem *b,
                void *aux);

/* Compare priorities of two threads */
bool
cond_higher_priority(const struct list_elem *a,
                     const struct list_elem *b,
                     void *aux);

/*Struct to store threads down waiting for wakeup_tick to reach*/
struct block_sema
{
  struct list_elem elem;
  int64_t wakeup_tick;		/* wake me up when this tick occurs */
  struct semaphore sema;  	/* semaphore to signal the thread to 
                             wake up at wakeup_tick */
};

/* Lock. */
struct lock 
  {
    struct thread *holder;      /* Thread holding lock (for debugging). */
    struct semaphore semaphore; /* Binary semaphore controlling access. */
  };

void lock_init (struct lock *);
void lock_acquire (struct lock *);
bool lock_try_acquire (struct lock *);
void lock_release (struct lock *);
bool lock_held_by_current_thread (const struct lock *);

void priority_donate(struct lock *);
/* Condition variable. */
struct condition 
  {
    struct list waiters;        /* List of waiting threads. */
  };

void cond_init (struct condition *);
void cond_wait (struct condition *, struct lock *);
void cond_signal (struct condition *, struct lock *);
void cond_broadcast (struct condition *, struct lock *);

/* Optimization barrier.

   The compiler will not reorder operations across an
   optimization barrier.  See "Optimization Barriers" in the
   reference guide for more information.*/
#define barrier() asm volatile ("" : : : "memory")

#endif /* threads/synch.h */
