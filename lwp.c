#include "lwp.h"
#include "scheduler.h"

#include <stdlib.h>

scheduler rsched;
unsigned long thread_count = 0;

tid_t lwp_create(lwpfun fun, void *arg, size_t size) {
    /* Create thread as local object */
    thread t = malloc(sizeof(context));
    t->tid = thread_count++;
    t->stack = malloc(size*sizeof(unsigned long));
    t->stacksize = size;

    /* Set up thread's rfile */
    t->state.rdi = (unsigned long) arg;
    t->state.rbp = (unsigned long) t->stack + (unsigned long) size;
    t->state.rsp = t->state.rbp;

    /* Push return address */
    unsigned long *sp = &t->state.rsp;
    sp--;
    *sp = (unsigned long) lwp_exit;

    /* Push old rbp -- this is a garbage value (?) because it's never used in
     * in lwp */
    sp--;
    *sp = (unsigned long) 0;

    /* Send thread context to scheduler */
    rsched->admit(t);

    /* return thread id */
    return t->tid;
}

void lwp_exit() {
	// TODO
}

tid_t lwp_gettid() {
    // TODO
}

void lwp_yield() {
    // TODO
}

void lwp_start() {
    /* Initialize default scheduler */
    rsched = malloc(sizeof(scheduler));
    rsched->init = NULL;
    rsched->shutdown = NULL;
    rsched->admit = &admit;
    rsched->remove = &remove;
    rsched->next = &next;
}

void lwp_stop() {
    // TODO
    // TODO : catch for if stop is called before start

    free(rsched);
}

void lwp_set_scheduler(scheduler sched) {
    // TODO
}

scheduler lwp_get_scheduler() {
    // TODO
}

thread tid2thread(tid_t tid) {
    // TODO
}
