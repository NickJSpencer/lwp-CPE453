#include "lwp.h"

#include <stdlib.h>

/* Default round-robin scheduler
 * Uses a linked list to keep track of threads */

static thread head = NULL;
static thread current = NULL;

void rr_admit(thread new) {
    /* Initialize linked list if it hasn't been initlialized yet */
    if (!head) {
        head = new;
        head->sched_one = NULL;
    }

    /* If the linked list has been initialized, create a new node
     * at the tail of the list */
    thread temp = head;
    while(temp->sched_one) {
        temp = temp->sched_one;
    }
    temp->sched_one = new;
    temp->sched_one->sched_one = NULL;
}

void rr_remove(thread victim) {
    /* TODO : catch for if victim == current? */

    /* If head hasn't been initialized yet, just return */
    if (!head) {
        return;
    }

    /* Catch for head collision */
    if (head->tid == victim->tid) {
        thread temp = head->sched_one;
        free(head);
        head = temp;
        return;
    }

    /* Iterate through the linked list */
    thread temp = head;
    while (temp->sched_one) {
        /* If we find the collision, delete the node and exit */
        if (temp->sched_one->tid == victim->tid) {
            thread next = temp->sched_one->sched_one;
            free(temp->sched_one);
            temp->sched_one = next;
            break;
        }

        /* Otherwise continue iterating through the linked list */
        temp = temp->sched_one;
    }
}

thread rr_next() {
    /* If there are no nodes in the list, simply return */
    if (!head) {
        return NULL;
    }

    /* If a current node hasn't been set yet or if the current node is the
     * tail of the linked list, set current to the head of the linked
     * list */
    if (!current || !current->sched_one) {
        current = head;
    }
    else {
        current = current->sched_one;
    }

    return current;
}

