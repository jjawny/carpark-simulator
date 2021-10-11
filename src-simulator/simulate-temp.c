/************************************************
 * @file    simulate-temp.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for simulate-temp.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <pthread.h>    /* for mutex locks/condition vars */
#include <stdlib.h>     /* for rand calls */

#include "simulate-temp.h"
#include "parking.h"    /* for shared memory types */
#include "sim-common.h" /* for args type/rand lock etc */
#include "sleep.h"      /* for milli sleep */

#define GREATEST(a,b) ((a>b) ? a:b)
#define SMALLEST(a,b) ((a<b) ? a:b)
#define DIFF(a,b) (a - b)

void *simulate_temp(void *args) {
    
    /* deconstruct args */
    args_t *a = (args_t *)args;
    level_t *lvl = (level_t*)((char *)shm + a->addr);

    int max;
    int min;
    int window;
    int rand_temp;
    int ms;

    /* ensure max temp is always greater than min */
    max = GREATEST(a->MAX_T, a->MIN_T); 
    min = SMALLEST(a->MIN_T, a->MAX_T);
    window = DIFF(max, min) + 1; /* +1 so rand can be min..max inclusive */
    //printf("id: %d max: %d, min: %d, diff: %d\n", a->id, max, min, window);
    
    while (!end_simulation) {
        /* get random temp & sleep duration */
        pthread_mutex_lock(&rand_lock);
        rand_temp = (rand() % window) + min;
        ms = (rand() % 5) + 1; /* 0..4 +1 = 1..5 */
        pthread_mutex_unlock(&rand_lock);

        /* apply random temp every 1..5 millis */
        sleep_for_millis(ms);
        lvl->temp_sensor = rand_temp;
    }
    
    free(a);
    return NULL;
}