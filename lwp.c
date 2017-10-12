#include "lwp.h"
#include "scheduler.h"

#include <stdlib.h>
#include <stdio.h>

static struct scheduler rsched = {NULL, NULL, &rr_admit, &rr_remove, &rr_next};
static unsigned long threadCount = 0;
static thread headThread = NULL;
static thread currentThread = NULL;
static rfile origRegs;

tid_t lwp_create(lwpfun fun, void *arg, size_t size) {

    /* Create thread as local object */
    thread t = malloc(sizeof(context));
    t->lib_one = NULL;
    t->tid = threadCount++;
    t->stack = malloc(size*sizeof(unsigned long));
    t->stacksize = size;

    /* Set up stack */
    unsigned long *sp = t->stack + size;
    sp--;
    *sp = (unsigned long) lwp_exit; /* return address */
    sp--;
    *sp = (unsigned long) fun;      /* thread's function */
    
    /* Set up thread's rfile */
    t->state.rdi = (unsigned long) arg;
    sp--;
    t->state.rbp = (unsigned long) sp;
    t->state.rsp = (unsigned long) sp;
    t->state.fxsave = FPU_INIT;
    
    /* Set up linked list of threads */
    if (!headThread) {
        headThread= t;
    }
    else {
        thread temp = headThread;
        while (temp->lib_one) {
            temp = temp->lib_one;
        }
        temp->lib_one = t;
    }

    /* Send thread context to scheduler */
    rsched.admit(t);
 
    /* Return thread id */
    return t->tid;
}

void lwp_exit() {
    // TODO : free stack
}

tid_t lwp_gettid() {
    return currentThread->tid;
}

void lwp_yield() {
    thread temp = currentThread;
    currentThread = rsched.next();
    if (!currentThread) {
        lwp_stop();
    }
    swap_rfiles(&temp->state, &currentThread->state);
}

void lwp_start() {
    thread next = rsched.next();
    if (next) {
        currentThread = next;
        swap_rfiles(&origRegs, &next->state);
    }
}

void lwp_stop() {
    // TODO
    // TODO : catch for if stop is called before start
}

void lwp_set_scheduler(scheduler sched) {
    // TODO
}

scheduler lwp_get_scheduler() {
    return rsched;
}

thread tid2thread(tid_t tid) {
    /* If linked list of threads hasn't been initialized yet */
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
