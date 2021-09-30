/************************************************
 * @file    simulator.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Main file for the Simulator software.
 *          Simulates hardware/cars in a carpark's
 *          lifecycle. Threads branch out from here.
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <stdlib.h>     /* for dynamic memory */
#include <string.h>     /* for string operations */
#include <unistd.h>     /* for misc */
#include <pthread.h>    /* for thread operations */
#include <sys/mman.h>   /* for shared memory operations */
#include <stdint.h>     /* for 16-bit integer type */

/* header APIs + read config file */
#include "spawn-cars.h"
#include "parking.h"
#include "queue.h"
#include "handle-entrance.h"
#include "sim-common.h"
#include "../config.h"

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920

/* init externs from "queue.h "*/
queue_t *en_queues[ENTRANCES];

/* init externs from "sim-common.h" */
_Atomic int end_simulation = 0; /* 0 = no, 1 = yes */
pthread_mutex_t en_queues_lock; 
pthread_cond_t en_queues_cond;
pthread_mutex_t rand_lock;

/**
 * @brief   Entry point for the SIMULATOR software.
 *          Initialises shared memory, queues, and other
 *          data structures. Branches off to other threads
 *          while the main thread waits until the program
 *          has run for 'n' amount of time before setting a
 *          global flag to tell all threads to finish their
 *          last cycle and join back here. Once all threads 
 *          have returned, Main will cleanup any leftover memory.
 * 
 * @param   argc - argument count, a standard param
 * @param   argv - arguments, a standard param
 * @return  int - indicating program's success or failure 
 */
int main (int argc, char **argv) {
    /* ---INITIALISE RANDOM SEED---
    Set to 1 for deterministic behaviour (debugging)
    Set to current time for true randomness */
    srand(time(NULL));

    /* ---CREATE SHARED MEMORY---
    will be unmapped at the end of main */
    void *shm = create_shared_memory(SHARED_MEM_NAME, SHARED_MEM_SIZE);
    init_shared_memory(shm, ENTRANCES, EXITS, LEVELS);
    
    /* ---CREATE n QUEUES FOR ENTRANCES---
    queue items will be freed at the end of main, 
    the queue itself freed in the corresponding entrance thread */
    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        queue_t *new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        en_queues[i] = new_q;
    }
    pthread_mutex_unlock(&en_queues_lock);

    /* ---START ENTRANCE THREADS---
    args will be freed within the relevant thread */
    pthread_t en_threads[ENTRANCES];

    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        /* set up args */
        en_args_t *args = malloc(sizeof(en_args_t) * 1);
        args->number = i;
        args->shared_memory = shm;
        args->queue = en_queues[i];

        pthread_create(&en_threads[i], NULL, handle_entrance, (void *)args);
    }
    pthread_mutex_unlock(&en_queues_lock);

    /*
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
    cars will be freed at the end of their lifecycle 
    (either at denied entry or from exit) */
    pthread_t spawn_cars_thread;
    pthread_create(&spawn_cars_thread, NULL, spawn_cars, NULL);

    /* ---ALERT ALL THREADS TO FINISH---
    after simulation has run for 'n' seconds */
    sleep(DURATION);
    end_simulation = 1;
    puts("Simulation ending, now cleaning up...");
    
    /* ---CLEANUP--- */
    /* wait for spawning-cars-thread to return before emptying the queues */
    pthread_join(spawn_cars_thread, NULL);

    /* empty the queues - free all items */
    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        empty_queue(en_queues[i]);
    }
    pthread_mutex_unlock(&en_queues_lock);
    puts("Queues destroyed");

    /* broadcast entrances to check their queues so they
    can exit, as they are currently waiting */
    pthread_cond_broadcast(&en_queues_cond);

    /* ---JOIN ALL THREADS BEFORE EXIT--- */
    for (int i = 0; i < ENTRANCES; i++) {
        pthread_join(en_threads[i], NULL);
    }
    //join exit and entrance threads here
    puts("All threads returned");

    destroy_shared_memory(shm, SHARED_MEM_SIZE, SHARED_MEM_NAME);

    return EXIT_SUCCESS;
}