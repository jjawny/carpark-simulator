/*******************************************************
 * @file    simulator.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Main file for the simulator software. Simulates
 *          the hardware/cars of a carpark lifecycle. All of 
 *          the simulator's header files link back here.
 ******************************************************/
#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <string.h>     /* for string stuff... */
#include <stdbool.h>    /* for bool stuff... */
#include <unistd.h>     /* misc */
#include <pthread.h>    /* for thread stuff */
#include <sys/mman.h>   /* for mapping shared like MAP_SHARED */
#include <stdint.h>     /* for 16-bit integer type */
#include <time.h>       /* for timing */

/* header APIs + read config file */
#include "parking.h"
#include "queue.h"
#include "../config.h"

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920

typedef struct en_args_t {
    int number;
    void *shared_memory;
    queue_t *queue;
} en_args_t;

/* function prototypes */
void *handle_entrance(void *args);

int main (int argc, char **argv) {
    
    /* create shared mem */
    /* shared mem will be unmapped at the end of main */
    void *shm = create_shared_memory(SHARED_MEM_NAME, SHARED_MEM_SIZE);
    init_shared_memory(shm, ENTRANCES, EXITS, LEVELS);
     
    /* create n queues for entrances */
    /* queues will be freed at the end of main */
    queue_t *all_queues[ENTRANCES];
    for (int i = 0; i < ENTRANCES; i++) {
        queue_t *new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        all_queues[i] = new_q;
    }

    /* ENTRANCE THREADS */
    pthread_t en_threads[ENTRANCES];
    for (int i = 0; i < ENTRANCES; i++) {
        /* args will be freed within relevant thread */
        en_args_t *args = malloc(sizeof(en_args_t) * 1);
        args->number = i;
        args->shared_memory = shm;
        args->queue = all_queues[i];

        pthread_create(&en_threads[i], NULL, handle_entrance, (void *)args);
    }

    /* create n cars to simulate */
    /* cars will be freed at the end of their lifecycle thread */
    for (int i = 0; i < TOTAL_CARS; i++) {
        car_t *new_c = malloc(sizeof(car_t) * 1);
        int totally_random = 1;
        strcpy(new_c->plate, "YEET69");
        push_queue(all_queues[totally_random], new_c);
    }
    //print_queue(all_queues[0]);

    /* join ALL threads before cleanup */
    for (int i = 0; i < ENTRANCES; i++) {
        pthread_join(en_threads[i], NULL);
    }

    /* unmap shared memory */
    destroy_shared_memory(shm, SHARED_MEM_SIZE, SHARED_MEM_NAME);

    /* destroy all queues */
    for (int i = 0; i < LEVELS; i++) {
        destroy_queue(all_queues[i]);
    }

    return EXIT_SUCCESS;
}

void *handle_entrance(void *args) {
    
    en_args_t *a = (en_args_t *)args;
    int floor = a->number;
    void *shm = a->shared_memory;
    queue_t *q = a->queue;
    
    /* grab associated shared memory data with this entrance */
    entrance_t *en = (entrance_t*)(shm + (sizeof(entrance_t) * floor));
    //printf("found en1's LPR plate thru arrows\t%s\n", en->sensor.plate);
    
    time_t startTime;
    time_t now;
    float elapsedTime;
    float setTime = 3.1;

    time(&startTime);
    while (elapsedTime < setTime) {
        sleep(1);
        car_t *next_car = pop_queue(q);
        if (next_car != NULL) {
            pthread_mutex_lock(&en->sensor.lock);
            strcmp(next_car->plate, en->sensor.plate);
            pthread_mutex_unlock(&en->sensor.lock);
            pthread_cond_signal(&en->sensor.condition);
        }
        printf("num plate: %s\n", next_car->plate);
        now = time(NULL);
        elapsedTime = difftime(now, startTime);
    }




    

    
    free(args);
    
    return NULL;
}

// gcc -o ../SIMULATOR simulator.c parking.c queue.c -Wall -lpthread -lrt
