/************************************************
 * @file    monitor-temp.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for monitor-temp.h
 ***********************************************/
#include <pthread.h> /* for mutex/condition types */
#include <unistd.h>  /* for misc like sleep */
#include <stdlib.h>  /* violates MISRA C and Nasa's The Power of 10 */

#include <stdio.h>   /* FOR DEMONSTRATION ONLY (print what triggers alarm) */

#include "fire-common.h"    /* common among fire alarm sys */
#include "monitor-temp.h"   /* corresponding header */

/* function prototypes */
void toggle_all_alarms(int active);
void delete_after(node_t *head, int after);
void free_nodes(node_t *head);
int bubble_sort(int *arr,int size);

void *monitor_temp(void *args) {
    
    /* deconstruct args to locate corresponding level */
    int id = *(int *)args;
    int addr = (int)((sizeof(entrance_t) * ENS) + (sizeof(exit_t) * EXS) + (sizeof(level_t) * id));
    level_t *l = (level_t *)((char *)shm + addr);

    /* define other variables here with default values */
    node_t *latest_temp = NULL;
    node_t *head = NULL;
    node_t *oldest_smoothed = NULL;
    node_t *latest_smoothed = NULL;
    node_t *smoothed_head = NULL;
    int count = 0;
    int highs = 0;

    /* -----------------------------------------------
     *       LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while(!end_simulation) {
        
        /* -----------------------------------------------
         *      ADD THE LATEST RAW TEMP TO
         *      THE LINKED LIST AS THE NEW HEAD
         * 
         *      ALSO DELETE NODES IF WE HAVE MORE THAN 5
         * -------------------------------------------- */
        latest_temp = malloc(sizeof(node_t) * 1);
        latest_temp->temp = l->temp_sensor;
        latest_temp->next = head;
        head = latest_temp;

        delete_after(head, MEDIAN_WINDOW);

        /* -----------------------------------------------
         *       COUNT HOW MANY RAW TEMPS WE HAVE
         * -------------------------------------------- */
        count = 0;
        for (node_t *n = head; n != NULL; n = n->next) count++;
            
        /* -----------------------------------------------
         *         ONCE WE HAVE 5 RAW TEMPS
         * -------------------------------------------- */
        if (count == MEDIAN_WINDOW) {
            int median;
            int raw_temps[MEDIAN_WINDOW];
            node_t *current = head;

            for (int i = 0; i < MEDIAN_WINDOW; i++) {
                if (current != NULL) {
                    raw_temps[i] = current->temp;
                    current = head->next;
                }
            }
            
            median = bubble_sort(raw_temps, MEDIAN_WINDOW);

            /* -----------------------------------------------
             *      ADD THE LATEST SMOOTHED TEMP TO
             *      THE LINKED LIST AS THE NEW HEAD
             * 
             *      ALSO DELETE NODES IF WE HAVE MORE THAN 30
             * -------------------------------------------- */
            latest_smoothed = malloc(sizeof(node_t) * 1);
            latest_smoothed->temp = median;
            latest_smoothed->next = smoothed_head;
            smoothed_head = latest_smoothed;

            delete_after(smoothed_head, SMOOTHED_WINDOW);

            /* -----------------------------------------------
             *      COUNT HOW MANY SMOOTHED TEMPS WE HAVE
             *      AND COUNT HOW MANY ARE 58+ DEGREES
             *      AND THE OLDEST TEMP
             * -------------------------------------------- */
            count = 0;
            highs = 0;
            for (node_t *n = smoothed_head; n != NULL; n = n->next) {
                /* Temperatures of 58+ degrees are a concern */
                if (n->temp >= 58) highs++;
                /* Store the oldest temperature for rate-of-rise detection */
                oldest_smoothed = n;
                count++;
            }

            /* -----------------------------------------------
             *         ONCE WE HAVE 30 SMOOTHED TEMPS
             * -------------------------------------------- */
            if (count == SMOOTHED_WINDOW) {

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


                    /* IO allowed for demonstration code
                    this will be removed in the production code */
                    printf("TRIGGERED RISE: 90%%+ of the smoothed temps are 58+°\n");

                    sleep(6); /* slow down constant looping if the alarm is already activated */
                }

                /* -----------------------------------------------
                 *                SPIKE ALGORITHM
                 * -----------------------------------------------
                 * If the newest temp is 8+ degrees higher than the oldest
                 * temp (out of the last 30), this is a high rate-of-rise.
                 * and alert EVACUATE sign and gate threads to wake up
                 */
                if (latest_smoothed->temp - oldest_smoothed->temp >= 8) {
                    pthread_mutex_lock(&alarm_m);
                    alarm_active = 1;
                    if(alarm_active) {
                        toggle_all_alarms(alarm_active); 
                    }
                    pthread_mutex_unlock(&alarm_m);
                    pthread_cond_broadcast(&alarm_c);


                    /* IO allowed for demonstration code
                    this will be removed in the production code */
                    printf("TRIGGERED SPIKE: %d° is 8+ degrees more than %d°\n", latest_smoothed->temp, oldest_smoothed->temp);

                    sleep(6); /* slow down constant looping if the alarm is already activated */
                }
            }
        }
        sleep_for_millis(2); /* collect temperatures every 2ms */
    }

    /* helper function will traverse linked-lists and free nodes */
    free_nodes(head);
    free_nodes(smoothed_head);
    return NULL;
}

void delete_after(node_t *head, int after) {
    int count = 1;
    node_t *curr = head->next;
    node_t *prev = head;

    /* only execute if "after" is at least 1 */
    if (after > 0) {
        while (curr != NULL) {
            count++;           
            node_t *temp = curr;
            curr = curr->next;

            /* free all nodes that come after "after" 
            otherwise keep track of the previous so
            when we begin freeing nodes, we can 
            prevent a dangling pointer */
            if (count > after) {
                prev->next = NULL; /* prevent dangling pointer */
                free(temp);
            } else {
                prev = temp;
            }
        }
    }
    return;
}

void free_nodes(node_t *head) {
    while (head != NULL) {
        node_t *temp = head;
        head = head->next;
        free(temp);
    }
    return;
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