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
#include <unistd.h>     /* for misc like sleep */
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
#define SHARED_MEM_SIZE 2920    /* size in bytes */

/* -----------------------------------------------
 *      INIT GLOBAL EXTERNS FROM sim-common.h
 * -------------------------------------------- */
volatile _Atomic int end_simulation = 0; /* 0 = no, 1 = yes */
void *shm;                      /* set once in main */
pthread_mutex_t rand_lock;      /* for rand calls */
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
    system("clear");
    puts("");
    puts("");
    puts("█▀ █ █▀▄▀█ █░█ █░░ ▄▀█ ▀█▀ █▀█ █▀█");
    puts("▄█ █ █░▀░█ █▄█ █▄▄ █▀█ ░█░ █▄█ █▀▄ - JM");
    printf("\nSTARTING SIMULATION...\n");
    puts("");

    /* -----------------------------------------------
     *  CHECK BOUNDS for FOR USER INPUTS IN config.h
     * -----------------------------------------------
     * As the number of ENTRANCES/CAPACITY/CHANCE etc are
     * subject to human error, bounds must be checked.
     * Check bounds here ONCE for simplicity.
     */
    int ENS = ENTRANCES;
    int EXS = EXITS;
    int LVLS = LEVELS;
    int CAP = CAPACITY;
    float CH = CHANCE;
    int DU = DURATION;
    int MIN_T = MIN_TEMP;
    int MAX_T = MAX_TEMP;

    puts("~Verifying ENTRANCES, EXITS, LEVELS are 1..5 inclusive...");
    if (ENTRANCES < 0 || ENTRANCES > 5) {
        ENS = 5;
        printf("\tENTRANCES out of bounds. Falling back to defaults (5)\n");
    }
    
    if (EXITS < 0 || EXITS > 5) {
        EXS = 5;
        printf("\tEXITS out of bounds. Falling back to defaults (5)\n");
    }
    
    if (LEVELS < 0 || LEVELS > 5) {
        LVLS = 5;
        printf("\tLEVELS out of bounds. Falling back to defaults (5)\n");
    }

    puts("~Verifying CAPACITY is greater than 0...");
    if (CAPACITY <= 0) {
        CAP = 20;
        printf("\tCAPACITY out of bounds. Falling back to defaults (20)\n");
    }
    
    puts("~Verifying CHANCE is 0..1 inclusive...");
    if (CHANCE < 0 || CHANCE > 1) {
        CH = 0.5;
        printf("\tCHANCE out of bounds. Falling back to defaults (50%%)\n");
    }

    puts("~Verifying DURATION is greater than 0...");
    if (DURATION < 0) {
        DU = 60;
        printf("\tDURATION out of bounds. Falling back to defaults (1 minute)\n");
    }

    puts("~Verifying TEMPERATURES are greater than 0 and MAX TEMP is greater than or equal to MIN TEMP...");
    if (MIN_TEMP > MAX_TEMP) {
        MIN_T = 27;
        MAX_T = 33;
        printf("\tMIN TEMP GREATER THAN MAX. Falling back to defaults (27..33 degrees)\n");
    }

    if (MIN_TEMP < 0) {
        MIN_T = 27;
        printf("\tMIN TEMPERATURE out of bounds. Falling back to defaults (27 degrees)\n");
    }

    if (MIN_TEMP < 0) {
        MAX_T = 33;
        printf("\tMAX TEMPERATURE out of bounds. Falling back to defaults (33 degrees)\n");
    }

    /* -----------------------------------------------
     *        INIT RAND's SEED (CURRENT TIME)
     * -----------------------------------------------
     * Set to 1 for deterministic behaviour (debugging)
     * Set to current time for true randomness
     */
    srand(time(NULL));

    /* -----------------------------------------------
     *           CREATE SHARED MEMORY OBJECT
     * -------------------------------------------- */
    shm = create_shared_memory(SHARED_MEM_NAME, SHARED_MEM_SIZE);
    init_shared_memory(shm, ENS, EXS, LVLS);
    puts("~Shared memory created/initialised");

    /* -----------------------------------------------
     *      CREATE QUEUES FOR ENTRANCES & EXITS
     * -------------------------------------------- */
    /* Allocate memory for queues */
    en_queues = malloc(sizeof(queue_t *) * ENS);
    ex_queues = malloc(sizeof(queue_t *) * EXS);

    queue_t *new_q;
    /* Create entrance queues */
    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENS; i++) {
        new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        en_queues[i] = new_q;
    }
    pthread_mutex_unlock(&en_queues_lock);

    /* Create exit queues */
    pthread_mutex_lock(&ex_queues_lock);
    for (int i = 0; i < EXS; i++) {
        new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        ex_queues[i] = new_q;
    }
    pthread_mutex_unlock(&ex_queues_lock);

    /* -----------------------------------------------
     *      START ENTRANCE & EXIT THREADS
     * -------------------------------------------- */
    pthread_t en_threads[ENS];
    pthread_t ex_threads[EXS];

    args_t *a;

    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENS; i++) {
        /* set up args - will be freed within their thread */
        a = malloc(sizeof(args_t) * 1);

        a->id = i;
        a->addr = (int)(sizeof(entrance_t) * i);
        a->ENS = ENS;
        a->EXS = EXS;
        a->LVLS = LVLS;
        a->CAP = CAP;
        a->CH = CH;
        a->car = NULL;
        a->queue = en_queues[i];

        pthread_create(&en_threads[i], NULL, simulate_entrance, (void *)a);
    }
    pthread_mutex_unlock(&en_queues_lock);

    pthread_mutex_lock(&ex_queues_lock);
    for (int i = 0; i < EXS; i++) {
        /* set up args - will be freed within their thread */
        a = malloc(sizeof(args_t) * 1);
        
        a->id = i;
        a->addr = (int)((sizeof(entrance_t) * ENS) + (sizeof(exit_t) * i));
        a->ENS = ENS;
        a->EXS = EXS;
        a->LVLS = LVLS;
        a->CAP = CAP;
        a->CH = CH;
        a->car = NULL;
        a->queue = ex_queues[i];

        pthread_create(&ex_threads[i], NULL, simulate_exit, (void *)a);
    }
    pthread_mutex_unlock(&ex_queues_lock);

    /* -----------------------------------------------
     *          START SPAWNING CARS THREAD
     * -----------------------------------------------
     * cars will be freed at the end of their lifecycle
     * (either at denied entry or from exit)
     */
    pthread_t spawn_cars_thread;

    /* set up args - will be freed within their thread */
    a = malloc(sizeof(args_t) * 1);

    a->id = 0;
    a->addr = 0;
    a->ENS = ENS;
    a->EXS = EXS;
    a->LVLS = LVLS;
    a->CAP = CAP;
    a->CH = CH;
    a->car = NULL;
    a->queue = NULL;

    pthread_create(&spawn_cars_thread, NULL, spawn_cars, (void *)a);

    printf("~You may now start the Manager & Fire Alarm System software...\n");

    /* -----------------------------------------------
     *          ALERT ALL THREADS TO FINISH
     * -------------------------------------------- */
    sleep(DU);
    end_simulation = 1;
    puts("~Simulation ending, now cleaning up...");
    
    /* -----------------------------------------------
     *                    EMPTY QUEUES
     * ---------------------------------------------*/
    /* wait for spawning-cars-thread to return before emptying the queues */
    pthread_join(spawn_cars_thread, NULL);

    /* empty the queues - FREE all cars */
    pthread_mutex_lock(&en_queues_lock);
    for (int i = 0; i < ENS; i++) empty_queue(en_queues[i]);
    pthread_mutex_unlock(&en_queues_lock);

    pthread_mutex_lock(&ex_queues_lock);
    for (int i = 0; i < EXS; i++) empty_queue(ex_queues[i]);
    pthread_mutex_unlock(&ex_queues_lock);

    /* -----------------------------------------------
     *                      CLEAN UP
     * -----------------------------------------------
     * broadcast all Queues to wake up entrances/exits threads so,
     * they can exit gracefully
     */
    pthread_cond_broadcast(&en_queues_cond);
    pthread_cond_broadcast(&ex_queues_cond);

    /* -----------------------------------------------
     *          JOIN ALL THREADS BEFORE EXIT
     * -------------------------------------------- */
    for (int i = 0; i < ENS; i++) pthread_join(en_threads[i], NULL);
    for (int i = 0; i < EXS; i++) pthread_join(ex_threads[i], NULL);
    puts("~All threads returned");

    /* -----------------------------------------------
     *          FREE QUEUES
     *          UNMAP SHARED MEMORY
     * -------------------------------------------- */
    free(a);
    free(new_q);
    free(en_queues);
    free(ex_queues);
    puts("~All queues destroyed");

    destroy_shared_memory(shm, SHARED_MEM_SIZE, SHARED_MEM_NAME);
    puts("~Shared memory unmapped");
    puts("~Goodbye");
    puts("");
    return EXIT_SUCCESS;
}