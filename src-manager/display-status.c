/************************************************
 * @file    display-status.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for display-status.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <stdlib.h>     /* for NULL & sysclear */
#include <string.h>     /* for string operations */
#include <time.h>       /* for sleeping */

#include "man-common.h" /* for carpark types */
#include "../config.h"  /* for no. of ENTRANCES/EXITS/LEVELS */

void *display(void *args) {
    int millis = 50;
    struct timespec remaining, requested = {millis / 1000, (millis % 1000) * 1000000};
    
    entrance_t *all_en[ENTRANCES];
    level_t *all_lvl[LEVELS];
    exit_t *all_ex[EXITS];

    for (int i = 0; i < ENTRANCES; i++) {
        all_en[i] = (entrance_t *)((char *)shm + (sizeof(entrance_t) * i));
    }
    for (int i = 0; i < LEVELS; i++) {
        all_lvl[i] = (level_t *)((char *)shm + (sizeof(entrance_t) * ENTRANCES) + (sizeof(exit_t) * EXITS) + (sizeof(level_t) * i));
    }
    for (int i = 0; i < EXITS; i++) {
        all_ex[i] = (exit_t *)((char *)shm + (sizeof(entrance_t) * ENTRANCES) + (sizeof(exit_t) * i));
    }
    
    /* After doing some research, since we are only READING the status 
    of the shared memory items, and we aren't worried about values changing
    while we are reading (as we want the raw values) there is no need to 
    lock with mutexes here */
    for (;;) {
        system("clear");
        puts("");
        puts("");
        puts("█▀▀ ▄▀█ █▀█ █▀█ ▄▀█ █▀█ █▄▀   █▀ ▀█▀ ▄▀█ ▀█▀ █░█ █▀");
        puts("█▄▄ █▀█ █▀▄ █▀▀ █▀█ █▀▄ █░█   ▄█ ░█░ █▀█ ░█░ █▄█ ▄█ -JM");
        puts("");

        /* print all entrances */
        for (int i = 0; i < ENTRANCES; i++) {
            printf("ENTRANCE #%d:\t", i + 1);

            if (strlen(all_en[i]->sensor.plate) < 6) {
                printf("LPR(------) ");
            } else {
                printf("LPR(%s) ", all_en[i]->sensor.plate);
            }

            printf("Gate(%c) ", all_en[i]->gate.status);

            if (all_en[i]->sign.display == 0) {
                printf("Sign(-)\n");
            } else {
                printf("Sign(%c)\n", all_en[i]->sign.display);
            }
        }

        /* print all exits */
        puts("");
        for (int i = 0; i < EXITS; i++) {
            printf("EXIT #%d:\t", i + 1);

            if (strlen(all_ex[i]->sensor.plate) < 6) {
                printf("LPR(------) ");
            } else {
                printf("LPR(%s) ", all_ex[i]->sensor.plate);
            }

            printf("Gate(%c)\n", all_ex[i]->gate.status);
        }

        /* print all levels */
        puts("");
        int total = 0;
        for (int i = 0; i < LEVELS; i++) {
            printf("LEVEL #%d:\t", i + 1);

            if (strlen(all_lvl[i]->sensor.plate) < 6) {
                printf("LPR(------) ");
            } else {
                printf("LPR(%s) ", all_lvl[i]->sensor.plate);
            }
            
            printf("Temp(%d°) ", 69);
            printf("Capacity(%d/%d)\n", curr_capacity[i], CAPACITY);
            total += curr_capacity[i];
        }

        /* print totals */
        printf("\n\t TOTAL CAPACITY: %d/%d parked", total, CAPACITY * LEVELS);
        printf("\n\tTOTAL CUSTOMERS: %d cars", total_cars_entered);
        printf("\n\t  TOTAL REVENUE: $%.2f\n", (float)revenue / 100);

        nanosleep(&requested, &remaining);
    }
    
    return NULL;
}