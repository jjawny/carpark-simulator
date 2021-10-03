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
#include "simulate-entrance.h"
#include "simulate-exit.h"
#include "sim-common.h"
#include "../config.h"

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920

/* INIT EXTERNS FROM "sim-common.h" */
_Atomic int end_simulation = 0; /* 0 = no, 1 = yes */
pthread_mutex_t rand_lock;      /* for rand calls */
void *shm;                      /* set once in main */
queue_t **en_queues;            /* entrance queues */
queue_t **ex_queues;            /* exit queues */
pthread_mutex_t en_queues_lock; 
pthread_mutex_t ex_queues_lock; 
pthread_cond_t en_queues_cond;
pthread_cond_t ex_queues_cond;

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

    /* {{{{{{{{{{INIT RANDOM SEED}}}}}}}}}} */
    /* Set to 1 for deterministic behaviour (debugging)
    Set to current time for true randomness */
    srand(time(NULL));

    /* {{{{{{{{{{CREATE SHARED MEMORY}}}}}}}}}} */
    shm = create_shared_memory(SHARED_MEM_NAME, SHARED_MEM_SIZE);
    init_shared_memory(shm, ENTRANCES, EXITS, LEVELS);
    
    /* {{{{{{{{{{CREATE QUEUES}}}}}}}}}} */
    /* Allocate memory for queues */
    en_queues = malloc(sizeof(queue_t *) * ENTRANCES);
    ex_queues = malloc(sizeof(queue_t *) * EXITS);

    /* Create entrance queues */
    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        queue_t *new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        en_queues[i] = new_q;
    }
    pthread_mutex_unlock(&en_queues_lock);

    /* Create exit queues */
    pthread_mutex_lock(&ex_queues_lock);
    for (int i = 0; i < EXITS; i++) {
        queue_t *new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        ex_queues[i] = new_q;
    }
    pthread_mutex_unlock(&ex_queues_lock);

    /* {{{{{{{{{{START ENTRANCE THREADS}}}}}}}}}} */
    pthread_t en_threads[ENTRANCES];

    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        /* set up args */
        args_t *a = malloc(sizeof(args_t) * 1);
        a->number = i;
        a->queue = en_queues[i];

        pthread_create(&en_threads[i], NULL, simulate_entrance, (void *)a);
    }
    pthread_mutex_unlock(&en_queues_lock);
    
    /* {{{{{{{{{{START EXIT THREADS}}}}}}}}}} */
    pthread_t ex_threads[EXITS];

    pthread_mutex_lock(&ex_queues_lock);
    for (int i = 0; i < EXITS; i++) {
        // set up args
        args_t *a = malloc(sizeof(args_t) * 1);
        a->number = i;
        a->queue = ex_queues[i];

        pthread_create(&ex_threads[i], NULL, simulate_exit, (void *)a);
    }
    pthread_mutex_unlock(&ex_queues_lock);

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
    for (int i = 0; i < ENTRANCES; i++) empty_queue(en_queues[i]);
    pthread_mutex_unlock(&en_queues_lock);

    pthread_mutex_lock(&ex_queues_lock);
    for (int i = 0; i < EXITS; i++) empty_queue(ex_queues[i]);
    pthread_mutex_unlock(&ex_queues_lock);

    free(en_queues);
    free(ex_queues);
    puts("Queues destroyed");

    /* broadcast entrances to check their queues so they
    can exit, as they are currently waiting */
    pthread_cond_broadcast(&en_queues_cond);
    pthread_cond_broadcast(&ex_queues_cond);

    /* ---JOIN ALL THREADS BEFORE EXIT--- */
    for (int i = 0; i < ENTRANCES; i++) pthread_join(en_threads[i], NULL);
    for (int i = 0; i < EXITS; i++) pthread_join(ex_threads[i], NULL);
    
    //join exit and entrance threads here
    puts("All threads returned");

    destroy_shared_memory(shm, SHARED_MEM_SIZE, SHARED_MEM_NAME);

    return EXIT_SUCCESS;
}