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

#include "manage-exit.h"
#include "plates-hash-table.h"
#include "man-common.h"
#include "../config.h"

/* function prototypes */
void write_file(char *name, char *plate, double bill);

void *manage_exit(void *args) {

    /* deconstruct args and locate corresponding shared memory */
    args_t *a = (args_t *)args;
    exit_t *ex = (exit_t *)((char *)shm + a->addr);
    //while (1) {}
    while (!end_simulation) {
        /* wait until Sim reads in license plate into LPR */
        pthread_mutex_lock(&ex->sensor.lock);
        if (strcmp(ex->sensor.plate, "") == 0) pthread_cond_wait(&ex->sensor.condition, &ex->sensor.lock);

        /* Gate is either closed or opened here */

        if (!end_simulation) {
            /* find plate's start time and calc the difference to bill,
            appending file or creating if it does not already exist,
            then unlock ASAP and broadcast so other threads may use */
            pthread_mutex_lock(&bill_ht_lock);
            node_t *car = hashtable_find(bill_ht, ex->sensor.plate);
            pthread_mutex_unlock(&bill_ht_lock);
            pthread_cond_broadcast(&bill_ht_cond);

            if (car != NULL) {
                /* CALCULATE BILL
                5c per millisecond*/
                uint64_t elapsed = 0;
                double bill = 0;
                struct timespec stop;
                clock_gettime(CLOCK_MONOTONIC_RAW, &stop);

                /* elapsed formula from https://stackoverflow.com/a/10192994 with modifications */
                elapsed = ((stop.tv_sec - car->start.tv_sec) * 1000000 + (stop.tv_nsec - car->start.tv_nsec) / 1000) / 1000;
                bill = (double)(elapsed * 5); /* divide 100 for dollars $$$ */

                /* APPEND BILLING FILE */
                //printf("HERE FOR %ld billed $%.2f\n", elapsed, bill /= 100);
                write_file("billing.txt", car->plate, bill);
                revenue = revenue + bill;

                /* UPDATE CAPACITY */
                pthread_mutex_lock(&curr_capacity_lock);
                curr_capacity[car->assigned_lvl]--;
                pthread_mutex_unlock(&curr_capacity_lock);
                pthread_cond_broadcast(&curr_capacity_cond);

                /* REMOVE NODE IN BILL # TABLE */
                /* in-case same car returns again */
                hashtable_delete(bill_ht, ex->sensor.plate);
            }

            /* RAISE GATE IF CLOSED */
            pthread_mutex_lock(&ex->gate.lock);
            if (ex->gate.status == 'C') ex->gate.status = 'R';
            pthread_mutex_unlock(&ex->gate.lock);
            pthread_cond_broadcast(&ex->gate.condition);

            strcpy(ex->sensor.plate, ""); /* reset LPR */
            pthread_mutex_unlock(&ex->sensor.lock);
        }
    }
    //puts("Exit returned");
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
    char line[1000]; /* buffer of text to append */
    
    /* format, append, close */
    sprintf(line, "%s $%.2f\n", plate, bill /= 100);
    fwrite(line, sizeof(char), strlen(line), fp);
    fclose(fp);
}