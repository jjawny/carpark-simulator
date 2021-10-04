/************************************************
 * @file    car-lifecycle.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for car-lifecycle.h
 ***********************************************/
#include <stdio.h>          /* for IO operations */
#include <pthread.h>        /* for mutex locks */
#include <stdlib.h>         /* for rand */
#include <string.h>         /* for string operations */

#include "car-lifecycle.h"  /* corresponding header */
#include "queue.h"          /* for joining exit queue */
#include "sleep.h"          /* for parking n milliseconds */
#include "parking.h"        /* for shared memory types */
#include "sim-common.h"     /* for the rand lock */
#include "../config.h"      /* for no. of EXITS */

void *car_lifecycle(void *car) {
    car_t *c = (car_t *)car;
    int stay = 0;
    int exit = 0;

    /* calculate address of level n */
    int addr = (int)((sizeof(entrance_t) * ENTRANCES) + (sizeof(exit_t) * EXITS) + (sizeof(level_t) * c->floor));
    level_t *lvl = (level_t*)((char *)shm + addr);

    /* lock rand ONCE here to grab all random values needed 
    so we can let other threads use rand ASAP */
    pthread_mutex_lock(&rand_lock);
    stay = (rand() % 9901) + 100; /* %9901 = 0..9900 and +100 = 100..10000 */
    exit = rand() % EXITS;
    pthread_mutex_unlock(&rand_lock);

    printf("%s will now park on floor %d for %dms\n", c->plate, c->floor + 1, stay);

    /* drive for 10ms to parking space,
    trigger LPR then park for 100..10000ms
    trigger LPR then drive for 10ms to random exit */
    sleep_for_millis(10);
    pthread_mutex_lock(&lvl->sensor.lock);
    strcpy(lvl->sensor.plate, c->plate);
    pthread_mutex_unlock(&lvl->sensor.lock);
    sleep_for_millis(stay);
    pthread_mutex_lock(&lvl->sensor.lock);
    strcpy(lvl->sensor.plate, c->plate);
    pthread_mutex_unlock(&lvl->sensor.lock);
    sleep_for_millis(10);

    /* queue up @ random exit */
    pthread_mutex_lock(&ex_queues_lock);
    push_queue(ex_queues[exit], c);
    pthread_mutex_unlock(&ex_queues_lock);
    pthread_cond_broadcast(&ex_queues_cond);

    /* thread ends here but car data flow continues to a random exit */
    return NULL;
}