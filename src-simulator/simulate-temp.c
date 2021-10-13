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

void *simulate_temp(void *args) {
    
    /* deconstruct args */
    args_t *a = (args_t *)args;
    level_t *lvl = (level_t*)((char *)shm + a->addr);

    /* ensure max temp is always greater than min, swap if needed */
    int temp = GREATEST(a->MAX_T, a->MIN_T); 
    a->MIN_T = SMALLEST(a->MIN_T, a->MAX_T);
    a->MAX_T = temp;

    int change = 1; /* how much the temperature can change up or down */
    int min_change;
    int max_change;
    int prev_temp = a->MIN_T;
    int rand_temp;
    int ms = 0;     /* generate new temp every few millis */

    while (!end_simulation) {

        /* set new window that temperature can change within, 
        if window falls out of bounds, bring back up to MIN_T
        and back down to MAX_T */
        if ((min_change = prev_temp - change) < a->MIN_T) min_change = a->MIN_T;
        if ((max_change = prev_temp + change) > a->MAX_T) max_change = a->MAX_T;

        /* get random temp & sleep duration */
        pthread_mutex_lock(&rand_lock);
        /* change temp up or down by...
        also max_change +1 to be ensure 0..max_change inclusive */
        rand_temp = rand() % ((max_change  - min_change) + 1);
        rand_temp += min_change;             /* bring temp back into bounds */
        ms = (rand() % 5) + 1;               /* 0..4 +1 = 1..5 */
        pthread_mutex_unlock(&rand_lock);

        /* apply random temp every 1..5 millis */
        sleep_for_millis(ms);
        lvl->temp_sensor = rand_temp;
        prev_temp = rand_temp;
    }
    free(a);
    return NULL;
}