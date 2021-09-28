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
#include "sleep.h"
#include "spawn-cars.h"
#include "parking.h"
#include "queue.h"
#include "handle-entrance.h"
#include "sim-common.h"
#include "../config.h"

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920

_Atomic int end_simulation = 0; // initially 0 (no), 1 = yes
queue_t *en_queues[ENTRANCES];
pthread_mutex_t en_queues_lock;        /* lock for list of queues as this is global */
pthread_mutex_t rand_lock;


/* function prototypes */


int main (int argc, char **argv) {
    /* ---INITIALISE RANDOM SEED---
    - set to 1 for deterministic behaviour (debugging)
    - set to current time for true randomness */
    srand(time(NULL));

    /* ---CREATE SHARED MEMORY---
    shared mem will be unmapped at the end of main */
    void *shm = create_shared_memory(SHARED_MEM_NAME, SHARED_MEM_SIZE);
    init_shared_memory(shm, ENTRANCES, EXITS, LEVELS);
    
    /* ---CREATE n QUEUES FOR ENTRANCES---
    all queues will be destroyed at the end of main */
    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        queue_t *new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        en_queues[i] = new_q;
    }
    pthread_mutex_unlock(&en_queues_lock);

    /* ---START ENTRANCE THREADS---
    arguments will be freed within the relevant thread */
    pthread_t en_threads[ENTRANCES];

    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        en_args_t *args = malloc(sizeof(en_args_t) * 1);
        args->number = i;
        args->shared_memory = shm;
        args->queue = en_queues[i];

        pthread_create(&en_threads[i], NULL, handle_entrance, (void *)args);
    }
    pthread_mutex_unlock(&en_queues_lock);

    /*
    // ---START LEVEL THREADS---
    pthread_t ex_threads[EXITS];
    
    for (int i = 0; i < EXITS; i++) {
        ex_args_t *args = malloc(sizeof(ex_args_t) * 1);
        args->number = i;
        args->shared_memory = shm;

        pthread_create(&ex_threads[i], NULL, handle_level, (void *)args);
    }

    // ---START EXIT THREADS---
    pthread_t lvl_threads[LEVELS];
    
    for (int i = 0; i < LEVELS; i++) {
        lvl_args_t *args = malloc(sizeof(lvl_args_t) * 1);
        args->number = i;
        args->shared_memory = shm;

        pthread_create(&lvl_threads[i], NULL, handle_exit, (void *)args);
    }
    */

    /* ---START SPAWNING CARS THREAD---
    cars will be freed at the end of their lifecycle (either at denied entry or from exit) */
    pthread_t spawn_cars_thread;
    pthread_create(&spawn_cars_thread, NULL, spawn_cars, NULL);


    /* ---ALERT ALL THREADS TO FINISH---
    after simulation has run for n seconds */
    sleep(DURATION);
    end_simulation = 1;
    puts("Simulation ending, now cleaning up...");
    
    /* ---JOIN ALL THREADS BEFORE CLEANUP--- */
    for (int i = 0; i < ENTRANCES; i++) {
        pthread_join(en_threads[i], NULL);
    }

    /* ---CLEANUP--- */

    /* unmap shared memory */
    destroy_shared_memory(shm, SHARED_MEM_SIZE, SHARED_MEM_NAME);

    /* destroy all queues */
    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < LEVELS; i++) {
        destroy_queue(en_queues[i]);
    }
    pthread_mutex_unlock(&en_queues_lock);
    puts("Queues destroyed");

    return EXIT_SUCCESS;
}



/* 
gcc -o ../SIMULATOR simulator.c parking.c queue.c sleep.c spawn-cars.c handle-entrance.c -Wall -lpthread -lrt
*/