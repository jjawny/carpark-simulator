/************************************************
 * @file    manage-entrance.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for manage-entrance.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <string.h>     /* for string operations */
#include <stdlib.h>     /* for misc */
#include <time.h>       /* for timespec/nanosleep */

#include "manage-entrance.h"
#include "plates-hash-table.h"
#include "man-common.h"
#include "../config.h"

void *manage_entrance(void *args) {

    /* -----------------------------------------------
     *    DECONSTRUCT ARGS & LOCATE SHARED ENTRANCE
     * -------------------------------------------- */
    args_t *a = (args_t *)args;
    entrance_t *en = (entrance_t*)((char *)shm + a->addr);

    /* The fire alarm sys will always set off ALL alarms, so only check 
    first level as there will always be at least 1 level */
    int addr = (int)((sizeof(entrance_t) * a->ENS) + (sizeof(exit_t) * a->EXS)); /* offset of where levels begin */
    level_t *lvl = (level_t *)((char *)shm + addr + (sizeof(level_t) * 0)); /* first level */


    /* -----------------------------------------------
     *       LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while (!end_simulation) {
        /* -----------------------------------------------
         *        WAIT FOR SIM TO READ PLATE INTO LPR
         * --------------------------------------------- */
        pthread_mutex_lock(&en->sensor.lock);
        while (strcmp(en->sensor.plate, "") == 0 && !end_simulation) {
            pthread_cond_wait(&en->sensor.condition, &en->sensor.lock);
        }

        /* Gate is either opened or closed by here - see SIMULATE-ENTRANCE.c */

        /* -----------------------------------------------
         *                 LOCK THE SIGN
         * -----------------------------------------------
         * Unlock sign after we've updated the display
         */
        pthread_mutex_lock(&en->sign.lock);

        /* -----------------------------------------------
         *  VERIFY CAR ONLY IF THE SIMULATION HASN'T ENDED
         *              AND THERE IS NO FIRE
         * -------------------------------------------- */
        if (!end_simulation && lvl->alarm != '1') {
            /* -----------------------------------------------
             *  VALIDATE LICENSE PLATE IN AUTHORISED # TABLE
             * -------------------------------------------- */
            pthread_mutex_lock(&auth_ht_lock);
            node_t *authorised = hashtable_find(auth_ht, en->sensor.plate);
            pthread_mutex_unlock(&auth_ht_lock);
            pthread_cond_broadcast(&auth_ht_cond);

            /* -----------------------------------------------
             *            LOCK THE BILLING # TABLE
             * -----------------------------------------------
             * To deal with 2 authorised cars trying to enter with the same license plate,
             * we will only allow one car in at a time (no duplicates). Once the car has
             * left the car park, the entry in the billing # table will have been deleted,
             * allowing the car to return, as if it is visiting again in real-life.
             */
            pthread_mutex_lock(&bill_ht_lock);
            node_t *dupe = hashtable_find(bill_ht, en->sensor.plate);

            /* -----------------------------------------------
             *       LOCK THE CURRENT CAPACITIES ARRAY
             * -----------------------------------------------
             * Grab the total capacity and do not unlock yet in-case the car enters and,
             * we need to update the current capacity for the assigned level
             */
            pthread_mutex_lock(&curr_capacity_lock);
            int total_cap = 0;
            for (int i = 0; i < a->LVLS; i++) {
                total_cap += curr_capacity[i];
            }

            /* -----------------------------------------------
             *            ASSIGNED FLOOR VARIABLE
             * -----------------------------------------------
             * Set to -1 because bounds will be checked,
             * so even if a car is authorised and the carpark
             * is NOT full, yet something goes wrong and
             * the Manager cannot find any spaces, this error
             * will be caught (carpark is full after all).
             * 
             * Also declared here in this outter scope as
             * after evaluating the car and updating the sign,
             * the fire alarm may jump in and change the sign
             * to EVACUATE causing the car to never enter, this
             * is how we handle that.
             */
            int assigned = 0; /* 0 = no, 1 = yes */
            int floor_to_goto = -1;

            /* -----------------------------------------------
             *    IF NOT AUTHORISED OR ALREADY IN CAR PARK
             * -------------------------------------------- */
            if (authorised == NULL || dupe != NULL) {
                en->sign.display = 'X';

            /* -----------------------------------------------
             *              IF CAR PARK IS FULL
             * -------------------------------------------- */
            } else if (total_cap >= (a->CAP * a->LVLS)) {
                en->sign.display = 'F';

            /* -----------------------------------------------
             *       IF AUTHORISED AND CAR PARK HAS SPACE
             * -------------------------------------------- */
            } else {
                /* check all levels for first available space but start
                with the corresponding level to this entrance */
                for (int i = 0; i < a->LVLS; i++) {
                    int temp = (i + a->id) % a->LVLS; /* equation to wrap around */
                    if (curr_capacity[temp] < a->CAP) {
                        curr_capacity[temp]++; /* reserve spot for car */
                        floor_to_goto = temp;
                        break;
                    }
                }

                /* check assigned floor bounds for safety */
                if (floor_to_goto >= 0 || floor_to_goto <= 5) {
                    /* add to billing # table with assigned floor
                    (function will add the current time) */
                    hashtable_add(bill_ht, en->sensor.plate, floor_to_goto);

                    /* set the sign's display to the assigned floor */
                    en->sign.display = (char)(floor_to_goto + '0');
                    total_cars_entered++;
                    /* -----------------------------------------------
                    *              RAISE GATE IF CLOSED
                    * -------------------------------------------- */
                    pthread_mutex_lock(&en->gate.lock);
                    if (en->gate.status == 'C') en->gate.status = 'R';
                    pthread_mutex_unlock(&en->gate.lock);
                    pthread_cond_broadcast(&en->gate.condition);

                    assigned = 1;

                } else {
                    /* safety check - carpark full after all */
                    en->sign.display = 'F';
                }
            }

            /* -----------------------------------------------
             *    UNLOCK BILLING # TABLE
             *    UNLOCK CURRENT CAPACITIES ARRAY
             *    UNLOCK SIGN
             * -------------------------------------------- */
            pthread_mutex_unlock(&bill_ht_lock);
            pthread_mutex_unlock(&curr_capacity_lock);

            /* Broadcast to Sim that sign has been updated
             * and Broadcast to all manager threads that the
             * billing # table and current capacities are available again */
            pthread_cond_broadcast(&bill_ht_cond);
            pthread_cond_broadcast(&curr_capacity_cond);

            /* IF the car was assigned a level but before the
            Sim could read the level (in sign), the fire alarm
            jumps in and changes it to EVACUATE... we de-assign
            the car as it never entered. */
            if (assigned && lvl->alarm == '1') {
                pthread_mutex_lock(&curr_capacity_lock);
                curr_capacity[floor_to_goto]--;
                pthread_mutex_unlock(&curr_capacity_lock);
            }
        }

        /* -----------------------------------------------
         *           RESET & UNLOCK THE LPR SENSOR
         *     ALSO LET OTHER THREAD DISPLAY THE STATUS
         * -----------------------------------------------
         * As we know by here the Sim is still waiting for
         * us to unlock/broadcast the sign, we can briefly
         * unlock & relock the LPR to allow the DISPLAY STATUS
         * thread to read the value before we clear it.
         * 
         * If we do this after we unlock the sign, there is
         * a chance that the Sim will continue to read in
         * the next plate into the LPR and THEN we may accidentally
         * clear that instead, causing a deadlock. Therefore,
         * this trick prevents deadlocks & race conditions.
         */
        pthread_mutex_unlock(&en->sensor.lock);
        
        /* 8 millisecond pause to allow other thread to read & display status of LPR */
        int millis = 8; 
        struct timespec remaining, requested = {(millis / 1000), ((millis % 1000) * 1000000)};
        nanosleep(&requested, &remaining);

        pthread_mutex_lock(&en->sensor.lock);
        strcpy(en->sensor.plate, "");
        pthread_mutex_unlock(&en->sensor.lock);

        /* -----------------------------------------------
         * UNLOCK & BROADCAST SIGN SO THAT SIM CHECKS IT
         * -------------------------------------------- */
        pthread_mutex_unlock(&en->sign.lock);
        pthread_cond_broadcast(&en->sign.condition);
    }
    free(a);
    return NULL;
}