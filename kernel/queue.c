
//  ==================================
// ||    Stephanie Briere Americo    ||
// ||          GRR20165313           ||
// ||     Sistemas Operacionais      ||
// ||          *PingPongOS           ||
// || Universidade Federal do Paraná ||
//  ==================================

#include "queue.h"
#include <stdio.h>

//  ==================================
// ||     FUNCTION: queue_append     ||
// ||                                ||
// || GOAL: Insert an element at the ||
// ||       the end of the queue.    ||
// ||                                ||
// || CONDITIONS:                    ||
// ||  - the queue must exist        ||
// ||  - the element must exist      ||
// ||  - the element must not be in  ||
// ||    another queue               ||
// ||                                ||
// || INPUT:                         ||
// ||  - queue                       ||
// ||  - element to be inserted in   ||
// ||    the queue                   ||
//  ==================================
void queue_append (queue_t **queue, queue_t *elem) {
    // Checking conditions...
    if (!queue) {
        fprintf(stderr, "QUEUE_APPEND: the queue does not exist.\n");
        return;
    }
    if (!elem) {
        fprintf(stderr, "QUEUE_APPEND: the element does not exist.\n");
        return;
    }
    if (elem->prev || elem->next) {
        fprintf(stderr, "QUEUE_APPEND: the element is already in a queue.\n");
        return;
    }

    if (*queue == NULL) { // empty queue, just insert in the head
        *queue = elem;
        elem->prev = elem;
        elem->next = elem;
    }
    else { // queue not empty, insert in the end
        // the prev of the (new) last element points to the (currently) last
        elem->prev = (*queue)->prev; 
        // the next of the (new) last element points to the first
        elem->next = (*queue);
        // adjusts the pointers of the (currently) last and the first element
        (*queue)->prev->next = elem;
        (*queue)->prev = elem;
    }
}

//  ==================================
// ||     FUNCTION: queue_remove     ||
// ||                                ||
// || GOAL: Removes the indicated    ||
// ||       element from the queue.  ||
// ||                                ||
// || CONDITIONS:                    ||
// ||  - the queue must exist        ||
// ||  - the queue can not be empty  ||
// ||  - the element must exist      ||
// ||  - the element must be in the  ||
// ||    queue                       ||
// ||                                ||
// || INPUT:                         ||
// ||  - queue                       ||
// ||  - element to be removed of    ||
// ||    the queue                   ||
// ||                                ||
// || RETURN:                        ||
// ||    Pointer to the removed      ||
// ||    element, or NULL if error.  ||
//  ==================================
queue_t *queue_remove (queue_t **queue, queue_t *elem) {
    // Checking conditions...
    if (!queue) {
        fprintf(stderr, "QUEUE_REMOVE: the queue does not exist.\n");
        return NULL;
    }
    if (*queue == NULL) {
        fprintf(stderr, "QUEUE_REMOVE: the queue is empty.\n");
        return NULL;
    }
    if (!elem) {
        fprintf(stderr, "QUEUE_REMOVE: the element does not exist.\n");
        return NULL;
    }

    queue_t *aux;

    // scrolls the queue looking for the element
    for (aux = *queue; aux != (*queue)->prev && aux != elem; aux = aux->next);

    if (aux != elem) {
        fprintf(stderr, "QUEUE_REMOVE: the element is not in the queue.\n");
        return NULL;
    }
    else { // element is in the queue
        if (*queue == (*queue)->prev) // queue_size is 1, so now is empty
            *queue = NULL;
        else if (*queue == elem) // elem is the head of queue, so the next becomes the head
            *queue = aux->next;

        // removes the element from the queue
        aux->prev->next = aux->next;
        aux->next->prev = aux->prev;

        aux->prev = NULL;
        aux->next = NULL;
        return aux;
    }
}

//  ==================================
// ||      FUNCTION: queue_size      ||
// ||                                ||
// || GOAL: Count the number of      ||
// ||       elements in the queue.   ||
// ||                                ||
// || INPUT:                         ||
// ||  - queue                       ||
// ||                                ||
// || RETURN:                        ||
// ||    Number of elements in the   ||
// ||    queue.                      ||
//  ==================================
int queue_size (queue_t *queue) {
    if (!queue) return 0; // empty queue

    int size = 1;
    for (queue_t *aux = queue; aux != queue->prev; aux = aux->next)
        ++size;

    return size;
}

//  ==================================
// ||     FUNCTION: queue_print      ||
// ||                                ||
// || GOAL: Scrolls the queue and    ||
// ||       print its contents.      ||
// ||                                ||
// || INPUT:                         ||
// ||  - queue name                  ||
// ||  - function that prints the    ||
// ||    queue element               ||
// ||  - queue                       ||
//  ==================================
void queue_print (char *name, queue_t *queue, void print_elem (void*)) {
    if (!queue) { // empty queue
        printf("%s: []\n", name);
        return;
    }

    printf("%s: [", name);
    print_elem(queue);
    
    for (queue_t *aux = queue->next; aux != queue; aux = aux->next) {
        printf(" ");
        print_elem(aux);
    }

    printf("]\n");
}
