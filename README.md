# Pintos-Project-1---Threads
Extended the functionality of a minimally functional thread system by implementing alarm clock, priority scheduler (with priority donation) and multilevel feedback queue scheduler.

+--------------------+
| CS 521             |
| PROJECT 1: THREADS |
| DESIGN DOCUMENT    |
+--------------------+
---- GROUP ----
>> Fill in the names and email addresses of your group members.
Team Arkos
Richie Verma <richieve@buffalo.edu>

---- PRELIMINARIES ----
>> If you have any preliminary comments on your submission, notes for the
>> TAs, or extra credit, please give them here.
>> Please cite any offline or online sources you consulted while
>> preparing your submission, other than the Pintos documentation, course
>> text, lecture notes, and course staff.

ALARM CLOCK
===========
---- DATA STRUCTURES ----
>> A1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration. Identify the purpose of each in 25 words or less.
-->synch.h
struct block_sema
{
struct list_elem elem;
int64_t wakeup_tick;
struct semaphore sema;
};
This is a new semaphore structure to store the wakeup_tick for a thread. This enables us to order the list of semaphores(for blocked threads) based on their wakeup time.
-->timer.c
static struct list blocked_semas : List to store threads waiting for wakeup_tick.
static struct lock blocked_semas_lock : Lock for the blocked_semas list. It regulates access for insertion/ deletion in the blocked_semas list.

---- ALGORITHMS ----
>> A2: Briefly describe what happens in a call to timer_sleep(),
>> including the effects of the timer interrupt handler.
In every call to timer_sleep, initialize a new semaphore block_sema for every thread and calculate and store the value of wakeup_tick for the thread. Then after acquiring the lock, this semaphore is inserted in the ordered list of waiting threads to be woken up. Then a sema_down is called for this thread and it is moved to blocked state.
In every call to timer_interrupt, the list of semaphores is traversed to check if the wakeup_tick is reached and then sema_up is called for the thread.

>> A3: What steps are taken to minimize the amount of time spent in
>> the timer interrupt handler?
To minimize time spent in the timer_interrupt handler, semaphores have been inserted in the list in an ordered way (ascending order of wakeup_tick). So when we traverse the list of semaphores and reach a point where current_tick is less than wakeup_tick, we stop the traversal.

---- SYNCHRONIZATION ----
>> A4: How are race conditions avoided when multiple threads call
>> timer_sleep() simultaneously?
Race condition can occur when multiple threads try to insert an element into the wait list. This is being avoided by using lock for the wait list in timer_sleep and lock_try_acquire in the timer_interrupt function. So, at a time only one thread can perform insertion on the wait_list and the other threads will be inserted in the waiters list for the lock. As soon as the process releases the lock, the next process in the waiters list of the lock can perform insertion in the list.

>> A5: How are race conditions avoided when a timer interrupt occurs
>> during a call to timer_sleep()?
Race condition can occur when a thread in timer_sleep tries to insert into blocked_semas and a thread in the timer_interrupt tries to delete from blocked_semas at the same time. This is being avoided by using lock for insertion in blocked_semas in timer_sleep and by using lock_try_acquire for deletion in blocked_semas in timer_interrupt. So,
at a time only one thread can acquire the lock for the blocked_semas list and perform insertion/deletion on the list.

---- RATIONALE ----
>> A6: Why did you choose this design? In what ways is it superior to
>> another design you considered?
All the threads are stored in the increasing order of their wake up tick in the list. This way, we do not need to traverse the complete list every time to wake up the threads. Thus we can say our algorithm is optimized wrt to time complexity.
Race conditions are avoided using locks instead of turning interrupts off.

PRIORITY SCHEDULING
===================
---- DATA STRUCTURES ----
>> B1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration. Identify the purpose of each in 25 words or less.
-->thread.h
struct thread
{
/*Priority Scheduling*/
int base_priority;
int flag_donation_received;
struct thread *donated_to_thread;
struct lock *donated_for_lock;
struct donation *donation_received_from;
}
*** Changed donated_to_threads list to one element of donated_to_thread since any thread can directly donate its priority to only one thread.
*** donation_received_from list has now been changed to a linked list since linked list manipulation is easier (removal and update)
*** intermediate_priority is now taken care of within the donation_received_from (donation) structure.
base_priority stores the original or base priority of thread, ie. the value before donation or the value to be assigned after all donations are released.
flag_donation_received is a flag that is initialized to 0. It is set to 1 after the thread’s priority changes once it receives donation from some other thread.
donated_to_thread has a reference to the thread to which the current thread has donated its priority to.
donated_for_lock stores a reference to the lock, acquiring which, lead to priority donation.
donation_received_from is a linked list to store the threads and their corresponding priorities from which the current thread has received priority from.
/*Structure to store details donation received:lock and priority */
struct donation
{
struct lock *dlock;
int dpriority;
struct donation *next;
};
*dlock stores reference of lock for which donation has been done
dpriority stores highest priority of all the donations received for this lock
*next is a reference to next donation received by same thread
-->thread.c
priority_donate() - When a thread tries to acquire a lock that is already being held by another thread with lower priority, then this function donates the priority of the current thread to the thread holding the lock recursively.
donation_release() - When a thread releases a lock, this function is called to release all the donated priority it has received from other threads and restore the base priority.
→ sync.c
struct semaphore_elem
{
int priority;
};
priority has been added to store the semaphore_elem’s in the order of decreasing priority in the waiters list for condition variables.

>> B2: Explain the data structure used to track priority donation.
>> Use ASCII art to diagram a nested donation. (Alternately, submit a
>> .png file.)

---- ALGORITHMS ----
>> B3: How do you ensure that the highest priority thread waiting for
>> a lock, semaphore, or condition variable wakes up first?
Every time a thread blocks acquiring a lock, semaphore or condition variable, it is added to the waiters list for the same. By ensuring that the waiters list is sorted before waking the threads up, threads are woken up in order of decreasing priority.

>> B4: Describe the sequence of events when a call to lock_acquire()
>> causes a priority donation. How is nested donation handled?
When a thread B tries to acquire a lock, it first checks if the lock is being held by another thread. If the lock is already being held by another thread A, then we check the A’s priority. If priority(A)<priority(B), then we call on the priority_donate function.
When priority_donate is called for thread B, then the following changes are done:
Thread A:
 flag_donation_received is set
 donations_received_from is stored with details of thread B’s priority and reference to current lock.
Thread B:
 donated_to_thread is updated with reference to thread A
 donated_for_lock is updated with reference to current lock
The priority donate function recursively assigns priority(B) to donated_to_thread of A and subsequently its donated_to_thread, thus taking care of nested donations. It also updates priority in donation structure corresponding to the lock.
Eventually every thread has only one donation and highest priority corresponding to a lock, even if it has received multiple donations for the same lock.
Once the lock has been released by thread A and acquired by thread B, the donation has been released. Hence for thread B, donated_to_thread and donated_for_lock has been reinitialised to NULL.

>> B5: Describe the sequence of events when lock_release() is called
>> on a lock that a higher-priority thread is waiting for.
Let’s assume thread A with priority 32 acquired two locks LA and LB. Thread B with priority 33 tries to acquire LB and since A’s priority is less than B’s priority, thread B donates its priority to thread A. Now when thread C with priority 34 tries to acquire LA, then thread C also donates its priority to thread A. Before releasing the lock, the 3 threads are in the following state:
Thread A:
base_priority->32
priority ->34
donated_to_thread -> NULL
donation_received_from -> LB/33 -> LA/34
flag_donation_received -> 1
Thread B:
base_priority->33
priority ->33
donated_to_thread -> thread A
donation_received_from -> NULL
flag_donation_received -> 0
Thread C:
base_priority->34
priority ->34
donated_to_threads -> thread A
donation_received_from -> NULL
flag_donation_received -> 0
Thread A is running and lock release is called on LA, then the donation entry corresponding lock A is deleted from thread A. Thread
A’s priority is then set to the highest priority in donations i.e for LB - 33.
Thread A:
base_priority->32
priority ->33
donated_to_thread -> NULL
donation_received_from -> LB/33
flag_donation_received -> 1
Thread A continues to run and when it releases lock LB, then again its donation (for LB) is removed. Since there is no other entry in the donations list, the flag_donation_received is set to 0 and the priority of thread A is set to the base_priority of thread A.

---- SYNCHRONIZATION ----
>> B6: Describe a potential race in thread_set_priority() and explain
>> how your implementation avoids it. Can you use a lock to avoid
>> this race?
Race condition can occur when a thread A tries to set its own priority using the thread_set_priority function and another thread B tries to set priority for thread A at the same time (in case of priority donation).
It can also occur after we set the new priority and then go on to check if the new priority is less than the priority of the first thread in the global ready_list of threads, and there is another thread inserting a thread in the same list.
We have avoided both these conditions by disabling interrupts since there is global data being manipulated between threads.

---- RATIONALE ----
>> B7: Why did you choose this design? In what ways is it superior to
>> another design you considered?
This design effectively manages the list of donations received for locks, and donations given as well. There is a limit on the
Fetching the highest priority thread from lists has been optimized by inserting threads in decreasing order of their priority. This way the time complexity is reduced as the first thread in the list always has highest priority.

ADVANCED SCHEDULER
==================
---- DATA STRUCTURES ----
>> C1: Copy here the declaration of each new or changed `struct' or
>> `struct' member, global or static variable, `typedef', or
>> enumeration. Identify the purpose of each in 25 words or less.
→ thread.c
struct thread
{
int recent_cpu;
int nice;
}
static int load_avg;
→ fixed-point.h
#define EPS_F 16384
#define TO_FIXED(n) ((n) * 16384)
#define TO_INT_Z(x) ((x) / 16384)
#define TO_INT_NEAR(x) ((x)>=0?(((x) + 8192) / 16384) : (((x) - 8192) / 16384))
#define ADD_X_Y(x,y) ((x) + (y))
#define ADD_X_N(x,n) ((x) + (n) * 16384)
#define SUB_X_Y(x,y) ((x) - (y))
#define SUB_X_N(x,n) ((x) - (n) * 16384)
#define MUL_X_Y(x,y) (((int64_t) (x)) * (y) / 16384)
#define MUL_X_N(x,n) ((x) * (n))
#define DIV_X_Y(x,y) (((int64_t) (x)) * 16384 / (y))
#define DIV_X_N(x,n) ((x) / (n))

---- ALGORITHMS ----
>> C2: Suppose threads A, B, and C have nice values 0, 1, and 2. Each
>> has a recent_cpu value of 0. Fill in the table below showing the
>> scheduling decision and the priority and recent_cpu values for each
>> thread after each given number of timer ticks:
timer recent_cpu priority thread ticks A B C A B C to run ----- -- -- -- -- -- -- ------ 0 0 0 0 63 61 59 A 4 4 0 0 62 61 59 A 8 8 0 0 61 61 59 B 12 8 4 0 61 60 59 A 16 12 4 0 60 60 59 B 20 12 8 0 60 59 59 A 24 16 8 0 59 59 59 C 28 16 8 4 59 59 58 B 32 16 12 4 59 58 58 A 36 20 12 4 58 58 58 C

>> C3: Did any ambiguities in the scheduler specification make values
>> in the table uncertain? If so, what rule did you use to resolve
>> them? Does this match the behavior of your scheduler?
Yes, the initial load_avg for the system is not provided and it has not been specified if the system state is a boot-up state (in which
case the load_average is initialized to 0). Also, the number of ready threads in the system has not been mentioned. All the calculations have been done assuming a load_average of 0 initially, and 3 ready threads in the system.

>> C4: How is the way you divided the cost of scheduling between code
>> inside and outside interrupt context likely to affect performance?
The interrupt handler should not take too long to execute, and is time-sensitive. Currently a lot of functionality has been implemented in the timer_interrupt function, which includes, calculation of priority and recent cpu for all threads and the load_avg of the system. It will increase the latency for the timer_interrupt and make the system slow.

---- RATIONALE ----
>> C5: Briefly critique your design, pointing out advantages and
>> disadvantages in your design choices. If you were to have extra
>> time to work on this part of the project, how might you choose to
>> refine or improve your design?
Advantages of this project’s design :
 Code is Modularized, easily readable and extensible in case of further use in upcoming projects.
 Threads can receive a donation from as many threads.
There is one limitation: threads can receive a donation corresponding to 8 locks only.
If we would have some extra time to work on this project, then more time could be spent on analysis of various function calls in which interrupts are being disabled and enabled so that use and corresponding repercussions of disabling the interrupts can be minimized.

>> C6: The assignment explains arithmetic for fixed-point math in
>> detail, but it leaves it open to you to implement it. Why did you
>> decide to implement it the way you did? If you created an
>> abstraction layer for fixed-point math, that is, an abstract data
>> type and/or a set of functions or macros to manipulate fixed-point
>> numbers, why did you do so? If not, why not?
Functionality of fixed-point math described in pintos manual has been implemented in a new header file as different functions to incorporate modularity, reusability, readability and extensibility in the project.
