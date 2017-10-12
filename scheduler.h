#include "lwp.h"

#include <stdlib.h>

/* Default round-robin scheduler
 * Uses a linked list to keep track of threads */
static thread head = NULL;
static thread current = NULL;

/* Add the passed context from the scheduler's scheduling pool */
void rr_admit(thread new) {
    /* Initialize linked list if it hasn't been initlialized yet */
    if (!head) {
        head = new;
        head->sched_one = NULL;
        head->sched_two = NULL;
        return;
    }

    /* Otherwise, iterate through the linked list til we reach the tail */
    thread temp = head;
    while(temp->sched_one) {
        temp = temp->sched_one;
    }
    /* Append the new thread to the tail of the linked list */
    new->sched_one = NULL;
    new->sched_two = temp;
    temp->sched_one = new;
}

/* Remove the passed context from the scheduler's scheduling pool */
void rr_remove(thread victim) {
    /* If head hasn't been initialized yet, just return */
    if (!head) {
        return;
    }

    /* If the thread we're removing is the head */
    if (head->tid == victim->tid) {
        /* If the thread we're removing is the head AND the current 
         * running thread */
        if (current == head) {
            /* Set the current thread to the tail of the linked list */
            while (current->sched_one) {
                current = current->sched_one;
            }
        }
        /* Set head to be the next thread in the linked list */
        head = head->sched_one;
        if (head) {
            head->sched_two = NULL;
        }
        return;
    }

    /* Rewire victim's adjacent threads */
    if (victim->sched_two) {
        victim->sched_two->sched_one = victim->sched_one;
    }
    if (victim->sched_one) {
        victim->sched_one->sched_two = victim->sched_two;
    }

    /* If the thread we're removing is the current running thread (but not 
     * head), set current to be an adjacent thread */
    if (victim == current) {
        if(victim->sched_two) {
            current = victim->sched_two;
        }
        else if (victim->sched_one) {
            current = victim->sched_one;
        }
    }

}

/* Return the next thread to be run or NULL if there isn't one */
thread rr_next() {
    /* If there are no nodes in the list, simply return */
    if (!head) {
        return NULL;
    }

    /* If current running thread hasn't been set yet or if the current node 
     * is the tail of the linked list, set current to the head of the linked
     * list */
    if (!current || !current->sched_one) {
        current = head;
    }
    else {
        current = current->sched_one;
    }

    return current;
}

