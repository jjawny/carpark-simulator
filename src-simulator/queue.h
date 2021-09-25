/*******************************************************
 * @file    queue.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for initialising, modifying, and destroying
 *          a queue. Queues being a line of cars waiting
 *          outside an entrance.
 ******************************************************/
#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct car_t {
    char plate[7];  /* 6 chars +1 for string null terminator */
    int floor;
    long duration;  /* milliseconds */
} car_t;

typedef struct node_t {
    car_t *car;             /* pointer to the car */
    struct node_t *next;    /* car behind */
} node_t;

typedef struct queue_t {
    node_t *head;   /* front of the line */
    node_t *tail;   /* back of the line */
} queue_t;

/**
 * Initialises a queue, as one must be initialised before use.
 * 
 * @param pointer to the queue
 */
void init_queue(queue_t *q);

/**
 * Pushes a car to the end of the queue.
 * 
 * @param pointer to the queue
 * @param car to push
 * @return true if car node was malloced properly, false otherwise
 */
bool push_queue(queue_t *q, car_t *c);

/**
 * Pops head of the queue. The first node will be deconstructed
 * and freed, returning only the car itself
 * 
 * @param pointer to the queue
 * @return car at the front of the queue
 */
car_t *pop_queue(queue_t *q);

/**
 * Prints all cars' license plates in a queue.
 * 
 * @param pointer to the queue
 */
void print_queue(queue_t *q);

/**
 * Destroys an entire queue. This involves free-ing each
 * node individually before free-ing the queue itself.
 * 
 * @param pointer to the queue
 */
void destroy_queue(queue_t *q);