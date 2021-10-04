/************************************************
 * @file    simulate-exit.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for simulate-exit.h
 ***********************************************/
#include <stdio.h>          /* for IO operations */
#include <stdlib.h>         /* for freeing & rand */
#include <string.h>         /* for string operations */
#include <pthread.h>        /* for multi-threading */

#include "simulate-exit.h"  /* corresponding header */
#include "sleep.h"          /* for boomgate timings */
#include "parking.h"        /* for shared memory types */
#include "queue.h"          /* for queue operations */
#include "sim-common.h"     /* for flag & rand lock */

void *simulate_exit(void *args) {

    /* deconstruct args and calculate address of exit n */
    args_t *a = (args_t *)args;
    queue_t *q = a->queue;
    int addr = (int)((sizeof(entrance_t) * ENTRANCES) + (sizeof(exit_t) * a->number));
    exit_t *ex = (exit_t*)((char *)shm + addr);

    pthread_mutex_lock(&ex->gate.lock);
    ex->gate.status = 'C'; /* only Sim can set boomgate to initially closed */
    pthread_mutex_unlock(&ex->gate.lock);

    while (!end_simulation) {

        /* Reason this is an IF rather than a WHILE is when the simulation has
        ended, Main will broadcast to all threads to check their queues, so they
        stop waiting and finish their cycle and return. As the head of the queue
        will be NULL by then (the queues are emptied for cleanup), if we use a
        WHILE, the threads will wait forever. But by using an IF, the threads can
        continue, and skip the rest of the cycle if the item popped is NULL */
        
        /* wait for a broadcast then check if there is a car waiting in the queue, 
        if so? pop, otherwise loop back here and wait again to prevent busy waiting */
        pthread_mutex_lock(&ex_queues_lock);
        if (q->head == NULL) pthread_cond_wait(&ex_queues_cond, &ex_queues_lock);
        car_t *c = pop_queue(q);
        pthread_mutex_unlock(&ex_queues_lock);

        /* check if gate needs to be closed */
        pthread_mutex_lock(&ex->gate.lock);
        if (ex->gate.status == 'L') {
            sleep_for_millis(10);
            ex->gate.status = 'C';
        }
        pthread_mutex_unlock(&ex->gate.lock);
        pthread_cond_broadcast(&ex->gate.condition);

        if (c != NULL) {
            /* immediately trigger LPR (spec does not say to wait) */
            pthread_mutex_lock(&ex->sensor.lock);
            strcpy(ex->sensor.plate, c->plate);
            pthread_mutex_unlock(&ex->sensor.lock);
            pthread_cond_signal(&ex->sensor.condition);

            /* BEGIN 3-WAY HANDSHAKE WITH MAN EXIT */
            /* wait for the manager to bill the car and start raising gate */
            pthread_mutex_lock(&ex->gate.lock);
            while (ex->gate.status == 'C') pthread_cond_wait(&ex->gate.condition, &ex->gate.lock);
            if (ex->gate.status == 'R') {
                sleep_for_millis(10);
                ex->gate.status = 'O';
            }
            
            free(c); /* car leaves Sim */

            /* unlock and signal manager ready to lower (reset) gate */
            pthread_mutex_unlock(&ex->gate.lock);
            pthread_cond_broadcast(&ex->gate.condition);

        }
    }
    free(args);
    return NULL;
}
