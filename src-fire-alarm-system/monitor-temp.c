/************************************************
 * @file    monitor-temp.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for monitor-temp.h
 ***********************************************/
#include <pthread.h> /* for mutex/condition types */
#include <unistd.h>  /* for misc like sleep */

#include "fire-common.h"    /* common among fire alarm sys */
#include "monitor-temp.h"   /* corresponding header */

/* function prototypes */
void toggle_all_alarms(int active);
int bubble_sort(int *arr,int size);

void *monitor_temp(void *args) {
    
    /* deconstruct args to locate corresponding level */
    int id = *(int *)args;
    int addr = (int)((sizeof(entrance_t) * ENS) + (sizeof(exit_t) * EXS) + (sizeof(level_t) * id));
    level_t *l = (level_t *)((char *)shm + addr);

    /* define other variables here with default values */
    int highs = 0;

    int temps[MEDIAN_WINDOW];
    int smoothed[SMOOTHED_WINDOW];

    for (int i = 0; i < 4; i++) {
        temps[i] = 0;
    }

    for (int i = 0; i < 29; i++) {
        smoothed[i] = 0;
    }

    /* -----------------------------------------------
     *       LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while(!end_simulation) {
        
        /* -----------------------------------------------
         * SHUFFLE TEMPS DOWN MANUALLY AS WE CANNOT USE MALLOC DUE TO MISRA C
         *                  ADD LATEST LEVEL TEMP
         * -------------------------------------------- */
        for (int i = 0; i < 4; i++) {
            temps[i] = temps[i + 1];
        }
        temps[4] = (int)l->temp_sensor;
            
        /* -----------------------------------------------
         *           ONCE WE HAVE 5 RAW TEMPS
         * AKA ONCE WE'VE READ 5 TEMPS, THEY WILL PUSH OUT
         *              ALL 0 PLACEHOLDERS
         * -------------------------------------------- */
        if (temps[0] != 0) {

            /* GET THE MEDIAN */
            int median = 0;           
            median = bubble_sort(temps, MEDIAN_WINDOW);

            /* -----------------------------------------------
             * SHUFFLE TEMPS DOWN MANUALLY AS WE CANNOT USE MALLOC DUE TO MISRA C
             *          ADD MEDIAN AS THE LATEST SMOOTHED TEMP
             * -------------------------------------------- */
            for (int i = 0; i < 29; i++) {
                smoothed[i] = smoothed[i + 1];
            }
            smoothed[29] = median;

            /* -----------------------------------------------
             * COUNT HOW MANY SMOOTHED TEMPS ARE 58+ DEGREES
             * -------------------------------------------- */
            for (int i = 0; i <= 29; i++) {
                if (smoothed[i] >= 58) highs++;
            }

            /* -----------------------------------------------
            *           ONCE WE HAVE 30 SMOOTHED TEMPS
            * AKA ONCE WE'VE SMOOTHED 30 TEMPS, THEY WILL PUSH OUT
            *              ALL 0 PLACEHOLDERS
            * -------------------------------------------- */
            if (smoothed[0] != 0) {

                /* -----------------------------------------------
                 *                RISE ALGORITHM
                 * -----------------------------------------------
                 * If 90% of the last 30 smoothed temps are 58+ degrees,
                 * this is considered a high temperature, activate alarm
                 * and alert EVACUATE sign and gate threads to wake up
                 */
                if (highs >= SMOOTHED_WINDOW * 0.9) {
                    pthread_mutex_lock(&alarm_m);
                    alarm_active = 1;
                    if(alarm_active) {
                        toggle_all_alarms(alarm_active);
                    }
                    pthread_mutex_unlock(&alarm_m);
                    pthread_cond_broadcast(&alarm_c);

                    /* print here "rise algorithm triggered" for demonstration only */

                    sleep(6); /* slow down constant looping if the alarm is already activated */
                }

                /* -----------------------------------------------
                 *                SPIKE ALGORITHM
                 * -----------------------------------------------
                 * If the newest temp is 8+ degrees higher than the oldest
                 * temp (out of the last 30), this is a high rate-of-rise.
                 * and alert EVACUATE sign and gate threads to wake up
                 */
                if (smoothed[29] - smoothed[0] >= 8) {
                    pthread_mutex_lock(&alarm_m);
                    alarm_active = 1;
                    if(alarm_active) {
                        toggle_all_alarms(alarm_active); 
                    }
                    pthread_mutex_unlock(&alarm_m);
                    pthread_cond_broadcast(&alarm_c);
                    
                    /* print here "spike algorithm triggered" for demonstration only */

                    sleep(6); /* slow down constant looping if the alarm is already activated */
                }
            }
        }
        sleep_for_millis(2); /* collect temperatures every 2ms */
    }

    return NULL;
}

int bubble_sort(int *arr, int size) { 
    int temp;

    for(int i = 0; i < (size - 1);i++) {
        for(int j = 0; j < (size - i - 1); j++) {
            if(arr[j] > arr[j+1]) {
             /* swap to keep ascending order */
             temp = arr[j];
             arr[j] = arr[j+1];
             arr[j+1] = temp;
            }
        }
    }

    return arr[(size - 1) / 2]; /* median */
}

void toggle_all_alarms(int active) {

    for (int i = 0; i < LVLS; i++) {
        int addr = (int)((sizeof(entrance_t) * ENS) + (sizeof(exit_t) * EXS) + (sizeof(level_t) * i));
        level_t *l = (level_t *)((char *)shm + addr);

        if (active) {
            l->alarm = '1';
        } else {
            l->alarm = '0';
        }
    }
}