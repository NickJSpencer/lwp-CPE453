#define ERROR -1

#include "lwp.h"
#include "scheduler.h"

#include <stdlib.h>
#include <stdio.h>

/* Default Round Robin scheduler */
static struct scheduler RoundRobin = {NULL, NULL, &rr_admit, &rr_remove, 
                                                                    &rr_next};
/* Pointer to scheduler used in lwp */
static scheduler rsched = &RoundRobin;

/* Used to keep track of thread id's */
static unsigned long threadCount = 0;

/* Head of linked list of threads */
static thread headThread = NULL;

/* Current thread being ran */
static thread currentThread = NULL;

/* Register file of main lwp process */
static rfile origRegs;

/* Helper function for preserving stack pointer when exiting a thread */
static void exitHelper();

/* Creates a new lightweight process which executes the given function with the
 * given argument. The new processes's stack will be stacksize words.
 * lwp_create() returns the (lightweight) thread id of the new thread or -1 if
 * the thread cannot be created */
tid_t lwp_create(lwpfun fun, void *arg, size_t size) {

    /* Create thread */
    thread newThread;
    if (!(newThread = calloc(1, sizeof(context)))) {
        perror("Error in calloc");
        return ERROR;
    }
    newThread->lib_one = NULL;
    newThread->tid = threadCount++;
    if (!(newThread->stack = calloc(size, sizeof(unsigned long)))) {
        perror("Error in calloc");
        return ERROR;
    }
    newThread->stacksize = size;

    /* Set up thread's stack */
    unsigned long *stackPointer = newThread->stack + size;
    stackPointer--;
    *stackPointer = (unsigned long) lwp_exit; /* return address */
    stackPointer--;
    *stackPointer = (unsigned long) fun;      /* thread's function */
    stackPointer--;

    /* Set up thread's register file */
    newThread->state.rdi = (unsigned long) arg;
    newThread->state.rbp = (unsigned long) stackPointer;
    newThread->state.rsp = (unsigned long) stackPointer;
    newThread->state.fxsave = FPU_INIT;
    
    /* Initalize linked list of threads if it doesn't already exist */
    if (!headThread) {
        headThread= newThread;
    }
    else {
        /* Otherwise, iterate through linked list */
        thread temp = headThread;
        while (temp->lib_one) {
            temp = temp->lib_one;
        }
        /* Append the new thread to the end of the linked list */
        if (newThread != temp) {
            temp->lib_one = newThread;
        }
    }

    /* Send thread context to the scheduler */
    rsched->admit(newThread);
 
    /* Return thread id */
    return newThread->tid;
}

/* Terminates the current LWP and frees its resources. Calls sched->next() to
 * get the next thread. If there are no other threads, restores the original
 * system thread */
void lwp_exit() {
    /* If there are no more threads, we can stop the lwp system */
    if (!headThread || !currentThread) {
        lwp_stop(); 
    }
    
    /* Remove the current thread from scheduler */
    rsched->remove(currentThread);

    /* Preserve thread's stack */
    SetSP(origRegs.rsp);

    /* Call exit helper to create a new stack */
    exitHelper();
}

/* Finishes removing a lwp 
 * Used as a separate function to preserve the lwp's rbp and rsp */
static void exitHelper() {
    /* Free thread */
    free(currentThread->stack);
    free(currentThread);
    
    /* Set the current thread to the next thread from the scheduler 
     * If the scheduler does not provide the next thread, then we can stop 
     * the lwp system */
    currentThread = rsched->next();
    if(!currentThread) {
        lwp_stop(); 
    }

    /* Load next thread's context into the physical registers */
    swap_rfiles(NULL, &currentThread->state);
}

/* Returns the tid of the calling LWP or NO.THREAD of not called by a LWP */
tid_t lwp_gettid() {
    if (!currentThread) {
        return NO_THREAD;
    }
    return currentThread->tid;
}

/* Yields control to another LWP. Which one depends on the scheduler. Saves the 
 * current LWP's context, picks the next one, restores that thread's context, 
 * and returns */
void lwp_yield() {
    /* Keep a pointer to the current thread that we're leaving behind */
    thread temp = currentThread;

    /* Get the next thread from the scheduler 
     * If the scheduler has no more threads, then we can end the LWP system */
    currentThread = rsched->next();
    if (!currentThread) {
        lwp_stop();
    }

    /* Otherwise, swap the previous thread's context with the new thread's
     * context */
    swap_rfiles(&temp->state, &currentThread->state);
}

/* Starts the LWP system. Saves the original context (for lwp_stop() to use 
 * later), picks a LWP and starts it running. If there are no LWP's, returns 
 * immediately */
void lwp_start() {
    /* Get a valid thread from the scheduler */
    thread next = rsched->next();
    if (next) {
        /* Set the current thread and load in its context */
        currentThread = next;
        swap_rfiles(&origRegs, &next->state);
    }
}

/* Stops the LWP system, restores the original stack pointer and returns to 
 * that context. (Wherever lwp_start() was called from. lwp_stop() does not 
 * destroy any existing contexts, and thread processing will be restarted
 * by a call to lwp_start() */
void lwp_stop() {
    /* If a current thread exists, swap its context out (to ensure it is 
     * preserved */
    if (currentThread) {
        swap_rfiles(&currentThread->state, &origRegs);
    }
    /* Otherwise just load in the original context */
    else {
        swap_rfiles(NULL, &origRegs);
    }

}

/* Causes the LWP package to use the given scheduler to choose the next process
 * to run. Transfers all threads from the old scheduler to the new one in
 * next() order. If scheduler is NULL the library should return to round-robin
 * scheduling */
void lwp_set_scheduler(scheduler sched) {
    /* Make sure sched is valid */
    if (!sched) {
        return;
    }

    /* If incoming scheduler has an init function, call it */
    if (sched->init) {
        sched->init();
    }

    /* Find the next thread from the outgoing scheduler */
    thread temp;
    while((temp = rsched->next())) {
        rsched->remove(temp);
        sched->admit(temp);
    }

    /* If outgoing scheduler has a shutdown function, call it */
    if (rsched->shutdown) {
        rsched->shutdown();
    }
    
    /* Set the new scheduler */
    rsched = sched;
}

/* Returns the pointer to the current scheduler */
scheduler lwp_get_scheduler() {
    return rsched;
}

/* Returns the thread corresponding to the given thread ID, or NULL if the ID
 * is invalid */
thread tid2thread(tid_t tid) {
    /* If linked list of threads hasn't been initialized yet or invalid tid */
    if (!headThread || tid < 0) {
        return NULL;
    }
    
    /* Iterate through linked list */
    thread temp = headThread;
    while(temp) {
        /* If we find the thread, return it */
        if (temp->tid == tid) {
            return temp;
        }
        temp = temp->lib_one;
    }
    
    /* We reached the tail of the linked list without finding a valid thread */
    return NULL;
}
