/************************************************
 * @file    manage-entrance.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for manage-entrance.h
 ***********************************************/
#include <stdbool.h>    /* for bool operations */
#include <time.h>       /* for time operations */
#include <sys/time.h>   /* for timeval type */

#include "manage-entrance.h"
#include "plates-hash-table.h"
#include "man-common.h"
#include "../config.h"

void *manage_entrance(void *args) {

    /* deconstruct args and locate corresponding shared memory */
    en_args_t *a = (en_args_t *)args;
    char *shm = a->shared_memory; /* cast to char for arithmetic */
    int addr = (sizeof(entrance_t) * a->number);
    entrance_t *en = (entrance_t*)(shm + addr);

    for (;;) {
        /* wait until Sim reads in license plate into LPR */
        pthread_mutex_lock(&en->sensor.lock);
        while (strcmp(en->sensor.plate, "") == 0) pthread_cond_wait(&en->sensor.condition, &en->sensor.lock);

        /* validate plate locking the authorised # table as it is global */
        pthread_mutex_lock(&auth_ht_lock);
        bool authorised = hashtable_find(auth_ht, en->sensor.plate);
        pthread_mutex_unlock(&auth_ht_lock);
        pthread_cond_broadcast(&auth_ht_cond);

        /* to deal with 2 authorised cars trying to enter with the same license plate,
        we will only allow one car as in at a time as if a car has left and returned
        in the real world (if both entered, one car would be an illegal vehicle). 
        We do not unlock until after the IF-ELSE blocks so that only one thread adds
        after confirming no duplicates - preventing future duplicates */
        pthread_mutex_lock(&bill_ht_lock);
        bool dupe = hashtable_find(bill_ht, en->sensor.plate);

        /* grab the total capacity and do not unlock until after the following IF-ELSE 
        blocks so (if authorised + not full) we can update the current capacity without 
        disrupting other threads */
        pthread_mutex_lock(&curr_capacity_lock);
        int total_cap = 0;
        for (int i = 0; i < LEVELS; i++) {
            total_cap += curr_capacity[i];
        }

        /* lock sign here so we can figure out the appropriate display to show the car */
        pthread_mutex_lock(&en->sign.lock);

        if (!authorised || dupe) {
            en->sign.display = 'X';

        } else if (total_cap >= (CAPACITY * LEVELS)) {
            en->sign.display = 'F';

        } else if (authorised) {
            /* find next avaliable floor as there is room somewhere */
            int floor_to_goto = 0;
            for (int i = 0; i < LEVELS; i++) {
                if (curr_capacity[i] < CAPACITY) {
                    curr_capacity[i]++; /* reserve spot for car */
                    floor_to_goto = i;
                    break;
                }
            }

            /* add to billing table with assigned floor 
            (function will note the current time added) 
            and set the sign's display to the assigned floor */
            hashtable_add(bill_ht, en->sensor.plate, floor_to_goto);
            en->sign.display = (char)floor_to_goto + '0';

            /* if gate closed? start raising,
            no need to signal as Sim will be waiting on sign */
            pthread_mutex_lock(&en->gate.lock);
            if (en->gate.status == 'C') en->gate.status = 'R';
            pthread_mutex_unlock(&en->gate.lock);
        }
        /* unlock billing # table, capacity, sign */
        pthread_mutex_unlock(&bill_ht_lock);
        pthread_mutex_unlock(&curr_capacity_lock);
        pthread_mutex_unlock(&en->sign.lock);

        /* signal Sim that sign has been updated and broadcast to all manager threads 
        that the billing # table and current capacities are available again */
        pthread_cond_signal(&en->sign.condition);
        pthread_cond_broadcast(&bill_ht_cond);
        pthread_cond_broadcast(&curr_capacity_cond);

        strcpy(en->sensor.plate, ""); /* reset LPR */
        pthread_mutex_unlock(&en->sensor.lock);

        /* 2 possibilities: 
        Either the gate is still raising (and Sim sets to open), it needs to be lowered (L)
        Or no car was let in and the gate remained closed (C) */
        pthread_mutex_lock(&en->gate.lock);
        while (en->gate.status == 'R') pthread_cond_wait(&en->gate.condition, &en->gate.lock);
        if (en->gate.status == 'O') en->gate.status = 'L';
        pthread_mutex_unlock(&en->gate.lock);
        pthread_cond_signal(&en->gate.condition);
//puts("completed cycle");
    }
    return NULL;
}




/*
struct timeval start, stop;
double secs = 0;

for (int i = 0; i < 5; i++) {
gettimeofday(&start, NULL);

   printf("Sleeping for milliseconds...\n");
           sleep_for(1000);

gettimeofday(&stop, NULL);
secs = (double)(stop.tv_usec - start.tv_usec) / 1000000 + (double)(stop.tv_sec - start.tv_sec);
printf("time taken %.2f\n",secs);
}*/