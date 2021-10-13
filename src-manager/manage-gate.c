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
    entrance_t *en = (entrance_t *)((char *)shm + a->addr);
    int opened; /* 0 = no, 1 = yes */
    
    /* The fire alarm sys will always set off ALL alarms, so only check 
    first level as there will always be at least 1 level */
    int addr = (int)((sizeof(entrance_t) * a->ENS) + (sizeof(exit_t) * a->EXS)); /* offset of where levels begin */
    level_t *lvl = (level_t *)((char *)shm + addr + (sizeof(level_t) * 0)); /* first level */

    while (!end_simulation) {
        opened = 0;

        /* wait until gate is opened */
        pthread_mutex_lock(&en->gate.lock);
        if (en->gate.status != 'O') pthread_cond_wait(&en->gate.condition, &en->gate.lock);
        if (en->gate.status == 'O') opened = 1;
        pthread_mutex_unlock(&en->gate.lock);

        /* If there's no fire and the gate is opened, keep open for 20ms before lowering */
        if (!end_simulation && lvl->alarm != '1' && opened) {
            sleep_for_millis(20);

            pthread_mutex_lock(&en->gate.lock);
            if (en->gate.status == 'O') en->gate.status = 'L';
            pthread_mutex_unlock(&en->gate.lock);
        }
    }
    return NULL;
}

void *manage_ex_gate(void *args) {

    /* deconstruct args and locate corresponding shared memory */
    args_t *a = (args_t *)args;
    exit_t *ex = (exit_t *)((char *)shm + a->addr);
    int opened; /* 0 = no, 1 = yes */

    /* The fire alarm sys will always set off ALL alarms, so only check 
    first level as there will always be at least 1 level */
    int addr = (int)((sizeof(entrance_t) * a->ENS) + (sizeof(exit_t) * a->EXS)); /* offset of where levels begin */
    level_t *lvl = (level_t *)((char *)shm + addr + (sizeof(level_t) * 0)); /* first level */
    
    while (!end_simulation) {
        opened = 0;

        /* wait until gate is opened */
        pthread_mutex_lock(&ex->gate.lock);
        if (ex->gate.status != 'O') pthread_cond_wait(&ex->gate.condition, &ex->gate.lock);
        if (ex->gate.status == 'O') opened = 1;
        pthread_mutex_unlock(&ex->gate.lock);

        /* If there's no fire and the gate is opened, keep open for 20ms before lowering */
        if (!end_simulation && lvl->alarm != '1' && opened) {
            sleep_for_millis(20);

            pthread_mutex_lock(&ex->gate.lock);
            if (ex->gate.status == 'O') ex->gate.status = 'L';
            pthread_mutex_unlock(&ex->gate.lock);
        }
    }
    return NULL;
}

void sleep_for_millis(int ms) {
    struct timespec remaining, requested = {(ms / 1000) * SLOW, ((ms % 1000) * 1000000) * SLOW};
    nanosleep(&requested, &remaining);
}