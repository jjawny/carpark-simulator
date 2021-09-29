/************************************************
 * @file    handle-entrance.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for handle-entrance.h
 ***********************************************/
#include "sleep.h"
#include "parking.h"
#include "queue.h"
#include "sim-common.h"
#include "handle-entrance.h"

#include "car-lifecycle.h" /* to send authorised cars off */

void *handle_entrance(void *args) {

    /* deconstruct args */
    en_args_t *a = (en_args_t *)args;
    int floor = a->number;
    char *shm = a->shared_memory;
    queue_t *q = a->queue; 

    /* locate corresponding shared memory data for this entrance */
    entrance_t *en = (entrance_t*)(shm + (sizeof(entrance_t) * floor));

    /* boomgate is always initially closed */
    en->gate.status = 'C';

    /* RUN UNTIL SIMULATION ENDS */
    while (!end_simulation) {

        /* Reason this is an IF rather than a WHILE is when the simulation
        has ended and Main broadcasts threads to check their queues so they 
        can finish their cycle and return, the head of the queue will always
        be NULL and therefore the threads will wait forever... by making this
        an IF statement, the queues will continue but will skip the rest of 
        the logic anyway if the head is still NULL.
        */

        /* wait for a broadcast to check if there is a car waiting in the queue,
        if so? grab it, otherwise loop back here and wait again to prevent busy waiting */
        pthread_mutex_lock(&en_queues_lock);
        if (q->head == NULL) pthread_cond_wait(&en_queues_cond, &en_queues_lock);
        car_t *c = pop_queue(q);
        pthread_mutex_unlock(&en_queues_lock);

        if (c != NULL) {
            /* wait 2 milliseconds before triggering LPR (read in plate) */
            sleep_for_millis(2);
            pthread_mutex_lock(&en->sensor.lock);
            strcpy(en->sensor.plate, c->plate);
            pthread_mutex_unlock(&en->sensor.lock);
            pthread_cond_signal(&en->sensor.condition);
            
            /* wait for the manager to validate plate and update sign */
            pthread_mutex_lock(&en->sign.lock);             /* SIGN IS LOCKED HERE */
            while (en->sign.display == 0) pthread_cond_wait(&en->sign.condition, &en->sign.lock);

            if (en->sign.display == 'X' || en->sign.display == 'F') {
                //printf("Car %s is NOT authorised! now leaving...\n", c->plate);
                free(c); /* car leaves simulation */
            } else {
                //printf("Car %s IS authorised! now entering...\n", c->plate);
                /* assign floor to car after converting to int */
                c->floor = (int)en->sign.display - '0';
                
                /* wait until gate raising or already opened to let car through */ 
                pthread_mutex_lock(&en->gate.lock);         /* GATE LOCKED HERE */
                while (en->gate.status != 'O' && en->gate.status != 'R') pthread_cond_wait(&en->gate.condition, &en->gate.lock);
                /* if raising? wait until its fully raised */
                if (en->gate.status == 'R') {
                    //puts("raising gate");
                    sleep_for_millis(10);
                    en->gate.status = 'O';
                }

                /* send cars off in their own thread as cars move independently once inside,
                starting a car's thread here rather than before joining the queue improves
                performance greatly, as only cars that are authorised/continuing are given 
                a thread, instead of every car */
                pthread_t new_car_thread;
                pthread_create(&new_car_thread, NULL, car_lifecycle, (void *)c);
                
                pthread_mutex_unlock(&en->gate.lock);       /* GATE UNLOCKED HERE */
                pthread_cond_signal(&en->gate.condition);   /* SIGNAL GATE IS OPENED */
            }
            pthread_mutex_unlock(&en->sign.lock);           /* SIGN IS UNLOCKED HERE */

            /* BOOM GATE LOCKED HERE */
            /* ensure the boom gate is closed before pulling up next car, 
            the manager will reset the info sign + LPR sensor */
            pthread_mutex_lock(&en->gate.lock);
            while (en->gate.status != 'C' && en->gate.status != 'L') pthread_cond_wait(&en->gate.condition, &en->gate.lock);
            
            /* if lowering? wait till fully lowered */
            if (en->gate.status == 'L') {
                //puts("lowering gate");
                sleep_for_millis(10);
                en->gate.status = 'C';
            }
            pthread_mutex_unlock(&en->gate.lock);
            /* BOOM GATE UNLOCKED HERE */

            /* after the gate is closed, clear the sign for the next car */
            pthread_mutex_lock(&en->sign.lock);
            en->sign.display = 0;
            pthread_mutex_unlock(&en->sign.lock);
        }
    }
    //puts("ending thread");
    free(args); /* queues are freed here */
    return NULL;
}
