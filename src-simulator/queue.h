#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>


typedef struct car_t {
    char plate[7]; /* 6 chars +1 for string null terminator */
    int floor;
    long duration; /* milliseconds */
} car_t;

typedef struct node_t {
    car_t *car; /* pointer to the car */
    struct node_t *next; /* behind? */
} node_t;

typedef struct queue_t {
    node_t *head;
    node_t *tail;
} queue_t;


void init_queue(queue_t *q);
bool push_queue(queue_t *q, car_t *c);
car_t *pop_queue(queue_t *q);
void print_queue(queue_t *q);
void destroy_queue(queue_t *q);