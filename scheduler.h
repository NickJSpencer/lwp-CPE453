#include "lwp.h"

#include <stdlib.h>

/* Default round-robin scheduler
 * Uses a linked list to keep track of threads */

typedef struct Node {
    thread t;
    struct Node *next;
} Node;

static Node *head, *current;

void init() {
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

void remove(thread victim) {
    /* TODO : catch for if victim == current? */

    /* If head hasn't been initialized yet, just return */
    if (!head) {
        return;
    }

    /* Catch for head collision */
    if (head->t->tid == victim->tid) {
        Node *temp = head->next;
        free(head);
        head = temp;
        return;
    }

    /* Iterate through the linked list */
    Node *temp = head;
    while (temp->next) {
        /* If we find the collision, delete the node and exit */
        if (temp->next->t->tid == victim->tid) {
            Node *next = temp->next->next;
            free(temp->next);
            temp->next = next;
            break;
        }

        /* Otherwise continue iterating through the linked list */
        temp = temp->next;
    }
}

thread next() {
    /* If there are no nodes in the list, simply return */
    if (!head) {
        return NULL;
    }

    /* If a current node hasn't been set yet or if the current node is the
     * tail of the linked list, set current to the head of the linked
     * list */
    if (!current || !current->next) {
        current = head;
    }
    else {
        current = current->next;
    }

    return current->t;
}

