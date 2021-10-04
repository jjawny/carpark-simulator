/************************************************
 * @file    manage-gate.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for manage-gate.h
 ***********************************************/
#include <stdio.h>   /* for IO operations */
#include <time.h>    /* for time operations */

#include "manage-gate.h"
#include "man-common.h"

/* function prototypes */
void sleep_for_millis(int ms);

void *manage_en_gate(void *args) {

    /* deconstruct args and locate corresponding shared memory */
    args_t *a = (args_t *)args;
    entrance_t *en = (entrance_t*)((char *)shm + a->addr);

    while (!end_simulation) {
        /* wait until gate is opened */
        pthread_mutex_lock(&en->gate.lock);
        if (en->gate.status != 'O') pthread_cond_wait(&en->gate.condition, &en->gate.lock);
        pthread_mutex_unlock(&en->gate.lock);
        pthread_cond_broadcast(&en->gate.condition);

        sleep_for_millis(20);
        if (en->gate.status == 'O') en->gate.status = 'L';
    }

    return NULL;
}

void *manage_ex_gate(void *args) {

    /* deconstruct args and locate corresponding shared memory */
    args_t *a = (args_t *)args;
    exit_t *ex = (exit_t *)((char *)shm + a->addr);

    while (!end_simulation) {
        /* wait until gate is opened */
        pthread_mutex_lock(&ex->gate.lock);
        if (ex->gate.status != 'O') pthread_cond_wait(&ex->gate.condition, &ex->gate.lock);
        pthread_mutex_unlock(&ex->gate.lock);
        pthread_cond_broadcast(&ex->gate.condition);

        sleep_for_millis(20);
        pthread_mutex_unlock(&ex->gate.lock);
        if (ex->gate.status == 'O') {
            ex->gate.status = 'L';
        }
        pthread_mutex_unlock(&ex->gate.lock);
        pthread_cond_broadcast(&ex->gate.condition);
    }

    return NULL;
}

void sleep_for_millis(int ms) {
    /* FOR DEMONSTRATION & DEBUGGING
     * slow down time (1 = no change) */
    long int scale = 1;

    struct timespec remaining, requested = {(ms / 1000) * scale, ((ms % 1000) * 1000000) * scale};
    nanosleep(&requested, &remaining);
}