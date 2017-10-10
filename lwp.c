#include "lwp.h"
#include "scheduler.h"

#include <stdlib.h>

scheduler rsched;

tid_t lwp_create(lwpfun fun, void *arg, size_t size) {
    // TODO
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
    rsched->init = &init;
    rsched->shutdown = NULL;
    rsched->admit = &admit;
    rsched->remove = &remove;
    rsched->next = &next;
}

void lwp_stop() {
    // TODO
    // TODO : catch for if stop is called before start
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
