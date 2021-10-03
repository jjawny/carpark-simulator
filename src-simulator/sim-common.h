/************************************************
 * @file    sim-common.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Common items among Simulator's seperate source files.
 ***********************************************/
#pragma once

#include "queue.h" /* for queue types */

/* ALL GLOBALS 
Rather than constantly passing pointers around 
(essentially making them global already) let them 
be global but restrict access using mutex locks */

extern _Atomic int end_simulation;  /* global flag - threads exit gracefully */
extern pthread_mutex_t rand_lock;   /* mutex lock - for rand calls as seed is global */
extern void *shm;                   /* pointer to first byte of shared memory */
extern queue_t **en_queues;         /* entrance queues */
extern queue_t **ex_queues;         /* exit queues */
extern pthread_mutex_t en_queues_lock;
extern pthread_mutex_t ex_queues_lock;
extern pthread_cond_t en_queues_cond;
extern pthread_cond_t ex_queues_cond;

/* entrance/exit/level thread args that include queues */
typedef struct args_t {
    int number;
    queue_t *queue;
} args_t;