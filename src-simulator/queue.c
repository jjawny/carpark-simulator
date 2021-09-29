/************************************************
 * @file    queue.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for queue.h
 ***********************************************/
#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <stdbool.h>    /* for bool stuff... */

#include "queue.h"      /* corresponding header */

void init_queue(queue_t *q) {
    q->head = NULL;
    q->tail = NULL;
}

bool push_queue(queue_t *q, car_t *c) {

    node_t *new_node = malloc(sizeof(node_t) * 1);
    new_node->car = c;
    new_node->next = NULL;

    if (new_node == NULL) {
        puts("malloc failed for adding node to queue");
        return false;
    }

    /* add car to the back of the line */
    if (q->tail != NULL) {
        q->tail->next = new_node;
    }
    q->tail = new_node;

    /* if the queue was empty, the car will also be the head */
    if (q->head == NULL) {
        q->head = new_node;
    }
    /* for debugging...
    puts("added to queue");
    */

    return true;
}

car_t *pop_queue(queue_t *q) {

    /* if queue is empty, abandon */
    if (q->head == NULL) {
        return NULL;
    }

    /* store the head so we can pop it and free it */
    node_t *temp = q->head;
    car_t *c = temp->car;

    /* pop it */
    q->head = q->head->next;
    if (q->head == NULL) {
        q->tail = NULL;
    }

    /* free it */
    free(temp);

    return c;
}

void print_queue(queue_t *q) {
    node_t *current = q->head;
    puts("Printing queue...");
    while (current != NULL) {
        printf("License plate:\t%s\n", current->car->plate);
        current = current->next;
    }
}

void empty_queue(queue_t *q) {
    node_t *current = q->head;
    while (current != NULL) {
        node_t *temp = current;
        current = current->next;
        free(temp);
    }
    q->head = NULL;
    q->tail = NULL;

    /* reason for not freeing queues here is because
    entrance/levels/exit threads wait on queues to have an item
    rather than looping to prevent busy waiting. Before exiting
    the program, Main will empty these queues and wake these threads 
    up (broadcast) to check if the head is NULL, and since the queues
    are still alive, they will see the head is NULL and be able to 
    finish their final loop and return to Main to exit gracefully. */
}