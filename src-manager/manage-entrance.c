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
    int floor = a->number;
    char *shm = a->shared_memory; /* char pointer for easy arithmetic */
    entrance_t *en = (entrance_t*)(shm + (sizeof(entrance_t) * floor));

    for (;;) {
        /* wait until LPR hardware reads in a license plate,
        once read, temporarily store the plate so we don't need 
        to constantly lock/unlock it for the rest of the cycle and
        clear the LPR ready for the next plate - if we clear later 
        there is a chance the Simulator may beat this Manager thread
        and read in the next number plate, so clearing ASAP prevents
        a deadlock*/
        pthread_mutex_lock(&en->sensor.lock);
        while (strcmp(en->sensor.plate, "") == 0) pthread_cond_wait(&en->sensor.condition, &en->sensor.lock);
        char temp_plate[7]; /* 6 chars +1 for NULL terminator */
        strcpy(temp_plate, en->sensor.plate);
        strcpy(en->sensor.plate, "");
        pthread_mutex_unlock(&en->sensor.lock);

        /* validate plate, locking the authorised #table as it is global */
        pthread_mutex_lock(&plates_ht_lock);
//printf("plate: %s\n", temp_plate);
        bool authorised = hashtable_find(plates_ht, temp_plate);
        pthread_mutex_unlock(&plates_ht_lock);

//if (authorised) puts("authorised!");

        /* to deal with 2 authorised cars trying to enter with the same license plate,
        we will only allow one car as in the real-world a duplicate would indicate an
        illegal vehicle. Therefore we lock the billing #table here so we can check for 
        duplicates, but we do NOT unlock until after the IF-ELSE blocks so that this is 
        the only thread inserting THIS car (to prevent duplicates) */
        pthread_mutex_lock(&bill_ht_lock);
        bool dupe = hashtable_find(bill_ht, temp_plate);

        /* grab the total capacity and do not unlock until after 
        the following IF-ELSE blocks so (if authorised + not full)
        we can update the current capacity without disrupting other
        threads */
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
                    curr_capacity[i]++; /* spot reserved for car */
                    floor_to_goto = i;
                    break;
                }
            }
            /* for debugging...
            printf("goto floor %c (as char), %d (as int)\n", floor_to_goto + '0', floor_to_goto);
            */

            /* add to billing table with assigned floor 
            (function will note the current time added) 
            and set the sign's display to the assigned floor */
            hashtable_add(bill_ht, temp_plate, floor_to_goto);
            en->sign.display = floor_to_goto + '0';
//puts("i am raising gate");
            /* if gate closed? start raising and signal
            simulator as it is waiting for gate to raise */
            pthread_mutex_lock(&en->gate.lock);
            if (en->gate.status == 'C') en->gate.status = 'R';
            pthread_mutex_unlock(&en->gate.lock);
            pthread_cond_signal(&en->gate.condition);
        }
        /* unlock current capacity, sign, and billing #table */
        pthread_mutex_unlock(&en->sign.lock);
        pthread_mutex_unlock(&curr_capacity_lock);
        pthread_mutex_unlock(&bill_ht_lock);

        /* signal simulator that sign has been updated 
        and broadcast to all manager threads that the
        current capacities are now avaliable again */
        pthread_cond_signal(&en->sign.condition);
        pthread_cond_broadcast(&curr_capacity_cond);

        /* wait while gate is still raising to prevent race condition,
        then if gate open? start lowering and signal sim as it is waiting
        for the gate to lower */
        pthread_mutex_lock(&en->gate.lock);
        while (en->gate.status == 'R') pthread_cond_wait(&en->gate.condition, &en->gate.lock);
        if (en->gate.status == 'O') en->gate.status = 'L';
        pthread_mutex_unlock(&en->gate.lock);
        pthread_cond_signal(&en->gate.condition);

        /* for debugging...
        puts("completed cycle");
        */
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