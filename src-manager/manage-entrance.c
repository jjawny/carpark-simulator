/************************************************
 * @file    manage-entrance.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for manage-entrance.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <stdlib.h>     /* for misc */

#include "manage-entrance.h"
#include "plates-hash-table.h"
#include "man-common.h"
#include "../config.h"

void *manage_entrance(void *args) {

    /* deconstruct args and locate corresponding shared memory */
    args_t *a = (args_t *)args;
    entrance_t *en = (entrance_t*)((char *)shm + a->addr);
    //while (1) {}
    while (!end_simulation) {
        /* wait until Sim reads in license plate into LPR */
        pthread_mutex_lock(&en->sensor.lock);
        if (strcmp(en->sensor.plate, "") == 0) pthread_cond_wait(&en->sensor.condition, &en->sensor.lock);

        /* Gate is either opened or closed by here */

        if (!end_simulation) {
            /* validate plate locking the authorised # table as it is global */
            pthread_mutex_lock(&auth_ht_lock);
            node_t *authorised = hashtable_find(auth_ht, en->sensor.plate);
            pthread_mutex_unlock(&auth_ht_lock);
            pthread_cond_broadcast(&auth_ht_cond);

            /* to deal with 2 authorised cars trying to enter with the same license plate,
            we will only allow one car as in at a time as if a car has left and returned
            in the real world (if both entered, one car would be an illegal vehicle).
            We do not unlock until after the IF-ELSE blocks so that only one thread adds
            after confirming no duplicates - preventing future duplicates */
            pthread_mutex_lock(&bill_ht_lock);
            node_t *dupe = hashtable_find(bill_ht, en->sensor.plate);

            /* grab the total capacity and do not unlock until after the following IF-ELSE
            blocks so (if authorised + not full) we can update the current capacity without
            disrupting other threads */
            pthread_mutex_lock(&curr_capacity_lock);
            int total_cap = 0;
            for (int i = 0; i < LEVELS; i++) {
                total_cap += curr_capacity[i];
            }

            /* lock sign here, so we can figure out the appropriate display to show the car */
            pthread_mutex_lock(&en->sign.lock);

            /* if not authorised or a duplicate (already in the carpark) */
            if (authorised == NULL || dupe != NULL) {
                en->sign.display = 'X';

            /* if authorised but carpark is full */
            } else if (total_cap >= (CAPACITY * LEVELS)) {
                en->sign.display = 'F';

            /* if authorised and carpark has space */
            } else {
                /* check all levels for first available space but start
                with the corresponding level with this entrance */
                int floor_to_goto = 0;
                for (int i = 0; i < LEVELS; i++) {
                    int temp = (i + a->id) % ENTRANCES; /* equation to wrap around */
                    if (curr_capacity[temp] < CAPACITY) {
                        curr_capacity[temp]++; /* reserve spot for car */
                        floor_to_goto = temp;
                        break;
                    }
                }

                /* add to billing table with assigned floor
                (function will note the current time added)
                and set the sign's display to the assigned floor */
                hashtable_add(bill_ht, en->sensor.plate, floor_to_goto);
                en->sign.display = (char)(floor_to_goto + '0');
                total_cars_entered++;

                /* if gate closed? start raising,
                no need to signal as Sim will be waiting on sign */
                pthread_mutex_lock(&en->gate.lock);
                if (en->gate.status == 'C') en->gate.status = 'R';
                pthread_mutex_unlock(&en->gate.lock);
                pthread_cond_broadcast(&en->gate.condition);
            }
            strcpy(en->sensor.plate, ""); /* reset LPR */
            pthread_mutex_unlock(&en->sensor.lock);

            /* unlock billing # table, capacity, sign */
            pthread_mutex_unlock(&bill_ht_lock);
            pthread_mutex_unlock(&curr_capacity_lock);
            pthread_mutex_unlock(&en->sign.lock);

            /* signal Sim that sign has been updated and broadcast to all manager threads
            that the billing # table and current capacities are available again */
            pthread_cond_signal(&en->sign.condition);
            pthread_cond_broadcast(&bill_ht_cond);
            pthread_cond_broadcast(&curr_capacity_cond);
        }
    }
    //puts("Entrance returned");
    free(a);
    return NULL;
}