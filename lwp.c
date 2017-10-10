#include "lwp.h"

#include <stdlib.h>

/***********************************************/
/* Default round-robin scheduler               *
 * Uses a linked list to keep track of threads */
/***********************************************/
typedef struct Node {
    thread t;
    struct Node *next;
} Node;

Node *head, *current;

void init(void) {
    /* Set head node and current node to be NULL */
    head = current = NULL;
}

void admit(thread new) {
    /* Initialize linked list if it hasn't been initlialized yet */
    if(!head) {
        head = malloc(sizeof(Node));
        head->t = new;
        head->next = NULL;
        return;
    }

    /* If the linked list has been initialized, create a new node
     * at the tail of the list */
    Node *temp = head;
    while(temp->next) {
        temp = temp->next;
    }
    temp->next = malloc(sizeof(Node));
    temp->next->t = new;
    temp->next->next = NULL;
}

void remove(thread v) {
	// TODO
}

thread next() {
	// TODO
}


/***********************************************/
/* LWP things                                  */
/***********************************************/
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
    rsched = malloc(sizeof(scheduler));
    rsched->init = &init;
    rsched->shutdown = NULL;
    rsched->admit = &admit;
    rsched->remove = &remove;
    rsched->next = &next;
}

void lwp_stop() {
    // TODO
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
