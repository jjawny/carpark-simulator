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
#include <time.h>               /* for timespec/nanosleep */

#include "simulate-entrance.h"  /* corresponding header */
#include "sleep.h"              /* for boomgate timing */
#include "parking.h"            /* for shared memory types */
#include "queue.h"              /* for queue operations */
#include "sim-common.h"         /* for flag & rand lock */
#include "car-lifecycle.h"      /* for sending authorised cars off */

void *simulate_entrance(void *args) {

    /* -----------------------------------------------
     *    DECONSTRUCT ARGS & LOCATE SHARED ENTRANCE
     * -----------------------------------------------
     * Freed at the end of this thread.
     */
    args_t *a = (args_t *)args;
    queue_t *q = a->queue;
    entrance_t *en = (entrance_t*)((char *)shm + a->addr);

    /* -----------------------------------------------
     *          ARGS FOR AUTHORISED CARS
     *        SENT OFF IN THEIR OWN THREADS
     * -----------------------------------------------
     * Freed at the end of this thread.
     */
    args_t *new_a = malloc(sizeof(args_t) * 1);
    
    new_a->id = a->id;
    new_a->addr = 0; /* will be changed to each car's assigned level */
    new_a->ENS = a->ENS;
    new_a->EXS = a->EXS;
    new_a->LVLS = a->LVLS;
    new_a->CAP = a->CAP;
    new_a->MIN_T = a->MIN_T;
    new_a->MAX_T = a->MAX_T;
    new_a->CH = a->CH;
    new_a->car = NULL; /* will be changed with each authorised car */
    new_a->queue = NULL;

    /* -----------------------------------------------
     *          GATE STARTS OFF CLOSED
     *          LPR STARTS OFF EMPTY
     *          SIGN STARTS OFF BLANK
     * -------------------------------------------- */
    pthread_mutex_lock(&en->gate.lock);
    en->gate.status = 'C';
    pthread_mutex_unlock(&en->gate.lock);

    pthread_mutex_lock(&en->sensor.lock);
    strcpy(en->sensor.plate, "");
    pthread_mutex_unlock(&en->sensor.lock);

    pthread_mutex_lock(&en->sign.lock);
    en->sign.display = 0;
    pthread_mutex_unlock(&en->sign.lock);

    /* -----------------------------------------------
     *      CREATE THREAD DETACHABLE ATTRIBUTE
     * -----------------------------------------------
     * As authorised cars that enter are sent off in
     * their own "CAR-LIFECYCLE" thread, these threads
     * must be detached upon creation so when they finish,
     * their memory can be freed/cleaned up immediately.
     * 
     * (detached instead of join as we cannot pause the
     * entrance to wait for each individual car to park)
     */
    pthread_attr_t detached;
    pthread_attr_init(&detached);
    pthread_attr_setdetachstate(&detached, PTHREAD_CREATE_DETACHED);

    /* -----------------------------------------------
     *       LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while (!end_simulation) {

        /* -----------------------------------------------
         *         WAIT UNTIL THERE'S A CAR WAITING
         * -------------------------------------------- */
        pthread_mutex_lock(&en_queues_lock);
        while (q->head == NULL && !end_simulation) pthread_cond_wait(&en_queues_cond, &en_queues_lock);
        car_t *c = pop_queue(q);
        pthread_mutex_unlock(&en_queues_lock);

        /* -----------------------------------------------
         *         CHECK IF GATE IS LOWERING
         *        (ONLY STAYS OPENED FOR 20ms)
         * -----------------------------------------------
         * Once opened, the gate will stay opened for 20ms 
         * then the Manager will lower the gate, and we will
         * close it here
         */
        pthread_mutex_lock(&en->gate.lock);
        if (en->gate.status == 'L') {
            sleep_for_millis(10);
            en->gate.status = 'C';
        }

        /* -----------------------------------------------
         *           OPEN GATE IF THERE IS A FIRE   
         * -----------------------------------------------
         * In the event of a fire, the gate will be raised
         * by the Fire Alarm System, and we will open it here
         */
        if (en->gate.status == 'R') {
            sleep_for_millis(10);
            en->gate.status = 'O';
        }
        pthread_mutex_unlock(&en->gate.lock);
        pthread_cond_broadcast(&en->gate.condition);


        if (c != NULL && !end_simulation) {
            /* -----------------------------------------------
             *      2ms BEFORE TRIGGERING LPR SENSOR
             * THEN UNLOCK & BROADCAST LPR SO MAN CHECKS IT
             * -------------------------------------------- */
            sleep_for_millis(2);
            pthread_mutex_lock(&en->sensor.lock);
            strcpy(en->sensor.plate, c->plate);
            pthread_mutex_unlock(&en->sensor.lock);

            /* 8 millisecond pause before we broadcast to the Manager
            that the LPR is ready, this is so that we can allow the 
            DISPLAY STATUS thread to read & display status of LPR */

            /* we DON'T use our custom sleep_for_millis function because the client can slow down
            time using the "SLOW MOTION" variable but we want this time to be constistent */
            int millis = 8; 
            struct timespec remaining, requested = {(millis / 1000), ((millis % 1000) * 1000000)};
            nanosleep(&requested, &remaining);

            pthread_cond_broadcast(&en->sensor.condition);

            /* -----------------------------------------------
             *      LOCK THE SIGN THEN
             *      WAIT FOR THE MANAGER TO VALIDATE PLATE
             *      AND UPDATE THE SIGN
             * -------------------------------------------- */
            pthread_mutex_lock(&en->sign.lock);
            while (en->sign.display == 0 && !end_simulation) pthread_cond_wait(&en->sign.condition, &en->sign.lock);

            /* -----------------------------------------------
             *      IF SIGN SAYS CAR IS...
             *          NOT AUTHORISED      (X)
             *          OR CAR PARK IS FULL (F)
             *          OR THERE'S A FIRE   (EVACUATE)
             * -------------------------------------------- */
            if (strchr("XFEVACUATE", en->sign.display) != NULL) {
                free(c); /* car leaves Sim */
            
            /* -----------------------------------------------
             *         IF AUTHORISED & ASSIGNED A LEVEL
             * -------------------------------------------- */
            } else if (!end_simulation) {
                c->floor = (int)en->sign.display - '0'; /* assign to floor */

                /* -----------------------------------------------
                 *        IF GATE IS CLOSED? WAIT FOR IT START RAISING
                 *        THEN FINISH RAISING AND OPEN (10ms)
                 *        THEN BROADCAST TO "MANAGE-GATE" THREADS
                 *        SO GATE STAYS OPEN FOR 20ms BEFORE LOWERING
                 * -------------------------------------------- */
                pthread_mutex_lock(&en->gate.lock);
                while (en->gate.status == 'C' && !end_simulation) pthread_cond_wait(&en->gate.condition, &en->gate.lock);
                if (en->gate.status == 'R') {
                    sleep_for_millis(10);
                    en->gate.status = 'O';
                }
                pthread_mutex_unlock(&en->gate.lock);
                pthread_cond_broadcast(&en->gate.condition);

                /* -----------------------------------------------
                 * SEND CARS OFF IN THEIR OWN "CAR-LIFECYCLE" THREAD
                 * -----------------------------------------------
                 * As cars move independently once inside.
                 */
                pthread_t new_car_thread;

                new_a->car = c;
                new_a->addr = (int)((sizeof(entrance_t) * a->ENS) + (sizeof(exit_t) * a->EXS) + (sizeof(level_t) * c->floor));
                
                pthread_create(&new_car_thread, &detached, car_lifecycle, (void *)new_a);
            }

            /* -----------------------------------------------
             *             RESET & UNLOCK THE SIGN
             * -------------------------------------------- */
            en->sign.display = 0; /* reset sign */
            pthread_mutex_unlock(&en->sign.lock);
        }
    }
    free(args);
    free(new_a);
    pthread_attr_destroy(&detached);
    return NULL;
}