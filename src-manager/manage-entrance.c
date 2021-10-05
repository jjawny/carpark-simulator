/************************************************
 * @file    manage-entrance.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for manage-entrance.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <string.h>     /* for string operations */
#include <stdlib.h>     /* for misc */

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

    /* -----------------------------------------------
     *       LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while (!end_simulation) {
        /* -----------------------------------------------
         *                 LOCK THE LPR SENSOR
         * -----------------------------------------------
         * Wait until Sim reads license plate into LPR, using
         * IF rather than WHILE so when simulation has ended,
         * Main can wake up these threads, and instead of waiting
         * again, threads can skip the rest of the loop and return
         */
        pthread_mutex_lock(&en->sensor.lock);
        if (strcmp(en->sensor.plate, "") == 0) pthread_cond_wait(&en->sensor.condition, &en->sensor.lock);

        /* Gate is either opened or closed by here */

        /* Check if the simulation has ended, if so? skip to the end */
        if (!end_simulation) {
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
            for (int i = 0; i < LEVELS; i++) {
                total_cap += curr_capacity[i];
            }

            /* -----------------------------------------------
             *                 LOCK THE SIGN
             * -----------------------------------------------
             * Unlock sign after we've updated the display
             */
            pthread_mutex_lock(&en->sign.lock);

            /* -----------------------------------------------
             *    IF NOT AUTHORISED OR ALREADY IN CAR PARK
             * -------------------------------------------- */
            if (authorised == NULL || dupe != NULL) {
                en->sign.display = 'X';

            /* -----------------------------------------------
             *              IF CAR PARK IS FULL
             * -------------------------------------------- */
            } else if (total_cap >= (CAPACITY * LEVELS)) {
                en->sign.display = 'F';

            /* -----------------------------------------------
             *       IF AUTHORISED AND CAR PARK HAS SPACE
             * -------------------------------------------- */
            } else {
                /* check all levels for first available space but start
                with the corresponding level to this entrance */
                int floor_to_goto = 0;
                for (int i = 0; i < LEVELS; i++) {
                    int temp = (i + a->id) % ENTRANCES; /* equation to wrap around */
                    if (curr_capacity[temp] < CAPACITY) {
                        curr_capacity[temp]++; /* reserve spot for car */
                        floor_to_goto = temp;
                        break;
                    }
                }

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
            }

            strcpy(en->sensor.plate, ""); /* reset LPR */

            /* -----------------------------------------------
             *    UNLOCK LPR SENSOR
             *    UNLOCK BILLING # TABLE
             *    UNLOCK CURRENT CAPACITIES ARRAY
             *    UNLOCK SIGN
             * -------------------------------------------- */
            pthread_mutex_unlock(&en->sensor.lock);
            pthread_mutex_unlock(&bill_ht_lock);
            pthread_mutex_unlock(&curr_capacity_lock);
            pthread_mutex_unlock(&en->sign.lock);

            /* Broadcast to Sim that sign has been updated
             * and Broadcast to all manager threads that the
             * billing # table and current capacities are available again */
            pthread_cond_broadcast(&en->sign.condition);
            pthread_cond_broadcast(&bill_ht_cond);
            pthread_cond_broadcast(&curr_capacity_cond);
        }
    }
    free(a);
    return NULL;
}