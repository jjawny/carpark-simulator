/************************************************
 * @file    display-status.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for display-status.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <stdlib.h>     /* for NULL & sys clear */
#include <string.h>     /* for string operations */
#include <pthread.h>    /* for mutex locking */
#include <time.h>       /* for sleeping */

#include "man-common.h" /* for car park types */
#include "../config.h"  /* for no. of ENTRANCES/EXITS/LEVELS */

void *display(void *args) {
    
    /* Deconstruct args */
    args_t *a = (args_t *)args;
    
    /* -----------------------------------------------
     *        SETUP TIMESPEC TO SLEEP FOR 50ms
     * -------------------------------------------- */
    int millis = 50;
    struct timespec remaining, requested = {(millis / 1000), ((millis % 1000) * 1000000)};

    /* -----------------------------------------------
     *     LOCATE ALL ENTRANCES, EXITS, & LEVELS
     * -------------------------------------------- */
    entrance_t *en[a->ENS];
    exit_t *ex[a->EXS];
    level_t *lvl[a->LVLS];

    for (int i = 0; i < a->ENS; i++) {
        en[i] = (entrance_t *)((char *)shm + (sizeof(entrance_t) * i));
    }
    for (int i = 0; i < a->EXS; i++) {
        ex[i] = (exit_t *)((char *)shm + (sizeof(entrance_t) * a->ENS) + (sizeof(exit_t) * i));
    }
    for (int i = 0; i < a->LVLS; i++) {
        lvl[i] = (level_t *)((char *)shm + (sizeof(entrance_t) * a->ENS) + (sizeof(exit_t) * a->EXS) + (sizeof(level_t) * i));
    }

    /* -----------------------------------------------
     *       LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while (!end_simulation) {
        system("clear");
        puts("");
        puts("");
        puts("█▀▀ ▄▀█ █▀█ █▀█ ▄▀█ █▀█ █▄▀   █▀ ▀█▀ ▄▀█ ▀█▀ █░█ █▀");
        puts("█▄▄ █▀█ █▀▄ █▀▀ █▀█ █▀▄ █░█   ▄█ ░█░ █▀█ ░█░ █▄█ ▄█ -JM");
        puts("");

        /* -----------------------------------------------
         *        PRINT STATUS OF ENTRANCE HARDWARE
         * -------------------------------------------- */
        for (int i = 0; i < a->ENS; i++) {
            printf("ENTRANCE #%d:\t", i + 1);

            pthread_mutex_lock(&en[i]->sensor.lock);
            if (strlen(en[i]->sensor.plate) < 6) {
                printf("LPR(------) ");
            } else {
                printf("LPR(%s) ", en[i]->sensor.plate);
            }
            pthread_mutex_unlock(&en[i]->sensor.lock);

            pthread_mutex_lock(&en[i]->gate.lock);
            printf("Gate(%c) ", en[i]->gate.status);
            pthread_mutex_unlock(&en[i]->gate.lock);

            pthread_mutex_lock(&en[i]->sign.lock);
            if (en[i]->sign.display == 0) {
                printf("Sign(-)\n");
            } else {
                printf("Sign(%c)\n", en[i]->sign.display);
            }
            pthread_mutex_unlock(&en[i]->sign.lock);
        }
        puts("");

        /* -----------------------------------------------
         *         PRINT STATUS OF EXIT HARDWARE
         * -------------------------------------------- */
        for (int i = 0; i < a->EXS; i++) {
            printf("EXIT #%d:\t", i + 1);

            pthread_mutex_lock(&ex[i]->sensor.lock);
            if (strlen(ex[i]->sensor.plate) < 6) {
                printf("LPR(------) ");
            } else {
                printf("LPR(%s) ", ex[i]->sensor.plate);
            }
            pthread_mutex_unlock(&ex[i]->sensor.lock);

            pthread_mutex_lock(&ex[i]->gate.lock);
            printf("Gate(%c)\n", ex[i]->gate.status);
            pthread_mutex_unlock(&ex[i]->gate.lock);
        }
        puts("");

        /* -----------------------------------------------
         *         PRINT STATUS OF LEVEL HARDWARE
         * -------------------------------------------- */
        int total = 0; /* kill 2 birds with 1 stone and get total here */
        for (int i = 0; i < a->LVLS; i++) {
            printf("LEVEL #%d:\t", i + 1);

            pthread_mutex_lock(&lvl[i]->sensor.lock);
            if (strlen(lvl[i]->sensor.plate) < 6) {
                printf("LPR(------) ");
            } else {
                printf("LPR(%s) ", lvl[i]->sensor.plate);
            }
            pthread_mutex_unlock(&lvl[i]->sensor.lock);

            printf("Temp(%d°) ", lvl[i]->temp_sensor); /* no mutex as temp is volatile */

            printf("Alarm(%c) ", lvl[i]->alarm); /* no mutex as alarm is volatile */

            pthread_mutex_lock(&curr_capacity_lock);
            printf("Capacity(%d/%d)parked\n", curr_capacity[i], a->CAP);
            total += curr_capacity[i];
            pthread_mutex_unlock(&curr_capacity_lock);
        }

        /* -----------------------------------------------
         *                  PRINT TOTALS
         * -------------------------------------------- */
        printf("\n\t TOTAL CAPACITY: %d/%d parked", total, a->CAP * a->LVLS);
        printf("\n\tTOTAL CUSTOMERS: %d cars", total_cars_entered);
        printf("\n\t  TOTAL REVENUE: $%.2f\n\n", (float)revenue / 100);

        /* -----------------------------------------------
         *              SLEEP FOR 50 MILLIS
         * -------------------------------------------- */
        nanosleep(&requested, &remaining);
    }
    free(a);
    return NULL;
}