/************************************************
 * @file    manage-exit.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for manage-exit.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <time.h>       /* for time operations */
#include <stdint.h>     /* for int types */
#include <stdlib.h>     /* for misc & clock */

#include "manage-exit.h"/* corresponding header */
#include "plates-hash-table.h"
#include "man-common.h"

/* function prototypes */
void write_file(char *name, char *plate, double bill);

void *manage_exit(void *args) {

    /* -----------------------------------------------
     *    DECONSTRUCT ARGS & LOCATE SHARED EXIT
     * -------------------------------------------- */
    args_t *a = (args_t *)args;
    exit_t *ex = (exit_t *)((char *)shm + a->addr);

    /* -----------------------------------------------
     *       LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while (!end_simulation) {
        /* -----------------------------------------------
         *        WAIT FOR SIM TO READ PLATE INTO LPR
         * --------------------------------------------- */
        pthread_mutex_lock(&ex->sensor.lock);
        while (strcmp(ex->sensor.plate, "") == 0 && !end_simulation) {
            pthread_cond_wait(&ex->sensor.condition, &ex->sensor.lock);
        }
        /* Gate is either opened or closed by here */

        /* Check if the simulation has ended, if so? skip to the end */
        if (!end_simulation) {
            /* find plate's start time and calc the difference to bill,
            appending file or creating if it does not already exist,
            then unlock ASAP and broadcast so other threads may use */
            pthread_mutex_lock(&bill_ht_lock);
            node_t *car = hashtable_find(bill_ht, ex->sensor.plate);
            pthread_mutex_unlock(&bill_ht_lock);
            pthread_cond_broadcast(&bill_ht_cond);

            if (car != NULL) {
                /* -----------------------------------------------
                 *          BILL CAR @ 5c PER MILLISECOND
                 * -------------------------------------------- */
                uint64_t elapsed = 0;
                double bill = 0;
                struct timespec stop;
                clock_gettime(CLOCK_MONOTONIC, &stop);

                /* elapsed formula from https://stackoverflow.com/a/10192994 with modifications */
                elapsed = ((stop.tv_sec - car->start.tv_sec) * 1000000 + (stop.tv_nsec - car->start.tv_nsec) / 1000) / 1000;
                bill = (double)(elapsed * 5); /* divide 100 for dollars $$$ */

                /* -----------------------------------------------
                 *               APPEND BILLING FILE
                 * -------------------------------------------- */
                write_file("billing.txt", car->plate, bill);
                revenue = revenue + bill;

                /* -----------------------------------------------
                 *             UPDATE CURRENT CAPACITY
                 * -------------------------------------------- */
                pthread_mutex_lock(&curr_capacity_lock);
                /* stay within bounds (at least 0) */
                if (curr_capacity[car->assigned_lvl] > 0) {
                    curr_capacity[car->assigned_lvl]--;
                }
                pthread_mutex_unlock(&curr_capacity_lock);
                pthread_cond_broadcast(&curr_capacity_cond);

                /* -----------------------------------------------
                 *          REMOVE CAR FROM BILLING # TABLE
                 * -----------------------------------------------
                 * in-case same car returns again
                 */
                pthread_mutex_lock(&bill_ht_lock);
                hashtable_delete(bill_ht, ex->sensor.plate);
                pthread_mutex_unlock(&bill_ht_lock);
                pthread_cond_broadcast(&bill_ht_cond);
            }

            /* -----------------------------------------------
             *             RAISE GATE IF CLOSED
             * -------------------------------------------- */
            pthread_mutex_lock(&ex->gate.lock);
            if (ex->gate.status == 'C') ex->gate.status = 'R';
            pthread_mutex_unlock(&ex->gate.lock);
            pthread_cond_broadcast(&ex->gate.condition);
        }
        /* -----------------------------------------------
         *           RESET & UNLOCK LPR SENSOR
         * -------------------------------------------- */
        strcpy(ex->sensor.plate, ""); /* reset LPR */
        pthread_mutex_unlock(&ex->sensor.lock);
    }
    free(a);
    return NULL;
}

void write_file(char *name, char *plate, double bill) {
    /* open file (or create if not exist) for appending */
    FILE *fp = fopen(name, "a");
    if (fp == NULL) {
        perror("fopen billing");
        exit(1);
    }
    char line[1000]; /* buffer for text to append */
    
    /* format, append, close */
    sprintf(line, "%s $%.2f\n", plate, bill /= 100);
    fwrite(line, sizeof(char), strlen(line), fp);
    fclose(fp);
}