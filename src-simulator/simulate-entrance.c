/************************************************
 * @file    simulate-entrance.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for simulate-entrance.h
 ***********************************************/
#include <stdio.h>              /* for IO operations */
#include <stdlib.h>             /* for freeing & rand */
#include <string.h>             /* for string operations */
#include <pthread.h>            /* for multi-threading */

#include "simulate-entrance.h"  /* corresponding header */
#include "sleep.h"              /* for boomgate timing */
#include "parking.h"            /* for shared memory types */
#include "queue.h"              /* for queue operations */
#include "sim-common.h"         /* for flag & rand lock */
#include "car-lifecycle.h"      /* for sending authorised cars off */

void *simulate_entrance(void *args) {

    /* deconstruct args and calculate address of entrance n */
    args_t *a = (args_t *)args;
    queue_t *q = a->queue;
    int addr = (int)(sizeof(entrance_t) * a->number);
    entrance_t *en = (entrance_t*)((char *)shm + addr);

    pthread_mutex_lock(&en->gate.lock);
    en->gate.status = 'C'; /* only sim can set boomgate to initially closed */
    pthread_mutex_unlock(&en->gate.lock);

    while (!end_simulation) {

        /* Reason this is an IF rather than a WHILE is when the simulation has
        ended, Main will broadcast to all threads to check their queues so they
        stop waiting and finish their cycle and return. As the head of the queue
        will be NULL by then (the queues are emptied for cleanup), if we use a
        WHILE, the threads will wait forever. But by using an IF, the threads can
        continue, and skip the rest of the cycle if the item popped is NULL */

        /* wait for a broadcast then check if there is a car waiting in the queue, 
        if so? pop, otherwise loop back here and wait again to prevent busy waiting */
        pthread_mutex_lock(&en_queues_lock);
        if (q->head == NULL) pthread_cond_wait(&en_queues_cond, &en_queues_lock);
        car_t *c = pop_queue(q);
        pthread_mutex_unlock(&en_queues_lock);

        if (c != NULL) {
            /* wait 2ms before triggering LPR */
            sleep_for_millis(2);
            pthread_mutex_lock(&en->sensor.lock);
            strcpy(en->sensor.plate, c->plate);
            pthread_mutex_unlock(&en->sensor.lock);
            pthread_cond_signal(&en->sensor.condition);

            /* wait for the manager to validate plate and update sign */
            pthread_mutex_lock(&en->sign.lock);
            while (en->sign.display == 0) pthread_cond_wait(&en->sign.condition, &en->sign.lock);

            if (en->sign.display == 'X' || en->sign.display == 'F') {
                free(c); /* car leaves Sim */
            } else {
                c->floor = (int)en->sign.display - '0'; /* assign to floor */

                /* if the car is authorised, the gates will be raising - this takes 10ms
                then we let the car through then signal the manager to close the gate */ 
                pthread_mutex_lock(&en->gate.lock);
                if (en->gate.status == 'R') {
                    sleep_for_millis(10);
                    en->gate.status = 'O';
                }

                /* send cars off in their own thread as cars move independently once inside,
                starting a car's thread here rather improves performance greatly, as only cars
                that are authorised/continuing are given a thread instead of every car */
                pthread_t new_car_thread;
                pthread_create(&new_car_thread, NULL, car_lifecycle, (void *)c);
                
                pthread_mutex_unlock(&en->gate.lock);
                pthread_cond_signal(&en->gate.condition);
            }
            pthread_mutex_unlock(&en->sign.lock);

            /* 2 possibilities: 
            Either the car had to leave the Sim and gate remained closed (C) 
            Or the car entered and the gate is left open (O) */
            pthread_mutex_lock(&en->gate.lock);
            while (en->gate.status == 'O') pthread_cond_wait(&en->gate.condition, &en->gate.lock);
            
            /* if car entered? gate will be lowering - this takes 10ms before closed */
            if (en->gate.status == 'L') {
                sleep_for_millis(10);
                en->gate.status = 'C';
            }
            pthread_mutex_unlock(&en->gate.lock);

            pthread_mutex_lock(&en->sign.lock);
            en->sign.display = 0; /* reset sign */
            pthread_mutex_unlock(&en->sign.lock);
        }
    }
    free(args);
    return NULL;
}
