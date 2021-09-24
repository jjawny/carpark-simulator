#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include "queue.h"

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
    puts("added to queue");

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

void destroy_queue(queue_t *q) {
    node_t *current = q->head;
    while (current != NULL) {
        node_t *temp = current;
        current = current->next;
        free(temp);
    }
    free(q);
    puts("Queue destroyed!");
}