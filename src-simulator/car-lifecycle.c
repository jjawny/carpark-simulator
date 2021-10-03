/************************************************
 * @file    car-lifecycle.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for car-lifecycle.h
 ***********************************************/
#include <stdio.h>          /* for IO operations */
#include <pthread.h>        /* for mutex locks */
#include <stdlib.h>         /* for rand */


#include "car-lifecycle.h"  /* corresponding header */
#include "queue.h"          /* for joining exit queue */
#include "sleep.h"          /* for parking n milliseconds */
#include "sim-common.h"     /* for the rand lock */
#include "../config.h"      /* for no. of EXITS */

void *car_lifecycle(void *car) {
    car_t *c = (car_t *)car;
    int stay = 0;
    int exit = 0;

    //printf("I %s have been told to goto floor: %d\n", c->plate, c->floor);

    /* lock rand ONCE here to grab all random values needed 
    so we can let other threads use rand ASAP */
    pthread_mutex_lock(&rand_lock);
    stay = (rand() % 9901) + 100; /* %9901 = 0..9900 and +100 = 100..10000 */
    exit = rand() % EXITS;
    pthread_mutex_unlock(&rand_lock);

    /* 10ms drive to parking space,
    then park for 100..10000ms
    then 10ms drive to random exit */
    sleep_for_millis(10);
    sleep_for_millis(stay); //read LPR before and after + join/leave lvl queue?
    sleep_for_millis(10);

    /* queue up @ random exit */
    pthread_mutex_lock(&ex_queues_lock);
    push_queue(ex_queues[exit], c);
    pthread_mutex_unlock(&ex_queues_lock);
    pthread_cond_broadcast(&ex_queues_cond);

    /* thread ends here but car data flow continues to a random exit */
    return NULL;
}