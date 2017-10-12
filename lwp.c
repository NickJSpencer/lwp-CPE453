#include "lwp.h"
#include "scheduler.h"

#include <stdlib.h>
#include <stdio.h>

static void exitHelper();
static struct scheduler RoundRobin = {NULL, NULL, &rr_admit, &rr_remove, 
                                                                    &rr_next};
static scheduler rsched = &RoundRobin;
static unsigned long threadCount = 0;
static thread headThread = NULL;
static thread currentThread = NULL;
static rfile origRegs;

tid_t lwp_create(lwpfun fun, void *arg, size_t size) {

    /* Create thread as local object */
    thread t = calloc(1, sizeof(context));
    t->lib_one = NULL;
    t->tid = threadCount++;
    t->stack = calloc(size, sizeof(unsigned long));
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
        if (t != temp) {
            temp->lib_one = t;
        }
    }

    /* Send thread context to scheduler */
    rsched->admit(t);
 
    /* Return thread id */
    return t->tid;
}

void lwp_exit() {
    if (!headThread || !currentThread) {
        lwp_stop(); // return?
    }
    
    rsched->remove(currentThread);
    SetSP(origRegs.rsp);
    exitHelper();
}

static void exitHelper() {
    free(currentThread->stack);
    free(currentThread);
    currentThread = rsched->next();
    if(!currentThread) {
        lwp_stop(); // return?
    }
    swap_rfiles(NULL, &currentThread->state);
}

tid_t lwp_gettid() {
    return currentThread->tid;
}

void lwp_yield() {
    thread temp = currentThread;
    currentThread = rsched->next();
    if (!currentThread) {
        lwp_stop();
    }
    swap_rfiles(&temp->state, &currentThread->state);
}

void lwp_start() {
    thread next = rsched->next();
    if (next) {
        currentThread = next;
        swap_rfiles(&origRegs, &next->state);
    }
}

void lwp_stop() {
    if (currentThread) {
        swap_rfiles(&currentThread->state, &origRegs);
    }
    else {
        swap_rfiles(NULL, &origRegs);
    }

}

void lwp_set_scheduler(scheduler sched) {
    if (!sched) {
        return;
    }

    if (sched->init) {
        sched->init();
    }

    thread t = rsched->next();
    while(t) {
        sched->admit(t);
        rsched->remove(t);
        t = rsched->next();
    }

    if (rsched->shutdown) {
        rsched->shutdown();
    }

    rsched = sched;
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
