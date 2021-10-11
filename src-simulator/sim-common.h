/************************************************
 * @file    sim-common.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Common items among Simulator's seperate source files.
 ***********************************************/
#pragma once

#include "queue.h" /* for queue types */

/* -----------------------------------------------
 *      ALL GLOBALS USED IN MANAGER SOFTWARE
 * -----------------------------------------------
 * Rather than constantly passing pointers around
 * (essentially making them global already) let them
 * be global but restrict access using mutex locks
 * and the _Atomic keyword
 *
 * All defined in Main (simulator.c)
 */
extern volatile _Atomic int end_simulation; /* global flag - threads exit gracefully */
extern void *shm;                           /* pointer to first byte of shared memory */
extern pthread_mutex_t rand_lock;           /* mutex lock - for rand calls as seed is global */
extern queue_t **en_queues;                 /* entrance queues */
extern queue_t **ex_queues;                 /* exit queues */
extern pthread_mutex_t en_queues_lock;
extern pthread_mutex_t ex_queues_lock;
extern pthread_cond_t en_queues_cond;
extern pthread_cond_t ex_queues_cond;

/* Thread args - a collection of commonly used values */
typedef struct args_t {
    int id;     /* to tell threads apart */
    int addr;   /* address of associated shared memory items */
    int ENS;    /* ENTRANCES after checking bounds */
    int EXS;    /* EXITS after checking bounds */
    int LVLS;   /* LEVELS after checking bounds */
    int CAP;    /* CAPACITY after checking bounds */
    float CH;   /* CHANCE after checking bounds */
    car_t *car; /* car for car-lifecycle threads */
    queue_t *queue; /* queues */
} args_t;