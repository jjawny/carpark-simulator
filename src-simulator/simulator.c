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
#include "sleep.h"
#include "../config.h"

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920

bool end_simulation;
queue_t *all_queues[ENTRANCES];

pthread_mutex_t end_simulation_lock;    /* lock for simulation finished flag */
pthread_mutex_t all_queues_lock;        /* lock for list of queues as this is global */
pthread_mutex_t rand_lock;              /* lock for rand calls as seed is global */

/* entrance thread args */
typedef struct en_args_t {
    int number;
    void *shared_memory;
    queue_t *queue;
} en_args_t;

/* function prototypes */
void random_plate(car_t *c);
void *spawn_cars(void *args);
void *handle_entrance(void *args);

int main (int argc, char **argv) {
    /* ---INITIALISE GLOBAL FLAG---
    that will end the simulation after n seconds */
    pthread_mutex_lock(&end_simulation_lock);
    end_simulation = false;
    pthread_mutex_unlock(&end_simulation_lock);

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
    pthread_mutex_lock(&all_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        queue_t *new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        all_queues[i] = new_q;
    }
    pthread_mutex_unlock(&all_queues_lock);

    /* ---START ENTRANCE THREADS---
    arguments will be freed within the relevant thread */
    pthread_t en_threads[ENTRANCES];

    pthread_mutex_lock(&all_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        en_args_t *args = malloc(sizeof(en_args_t) * 1);
        args->number = i;
        args->shared_memory = shm;
        args->queue = all_queues[i];

        pthread_create(&en_threads[i], NULL, handle_entrance, (void *)args);
    }
    pthread_mutex_unlock(&all_queues_lock);

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
    pthread_mutex_lock(&end_simulation_lock);
    sleep(DURATION);
    end_simulation = true;
    pthread_mutex_unlock(&end_simulation_lock);
    puts("Simulation ending, now cleaning up...");
    
    /* ---JOIN ALL THREADS BEFORE CLEANUP--- */
    for (int i = 0; i < ENTRANCES; i++) {
        pthread_join(en_threads[i], NULL);
    }

    /* ---CLEANUP--- */

    /* unmap shared memory */
    destroy_shared_memory(shm, SHARED_MEM_SIZE, SHARED_MEM_NAME);

    /* destroy all queues */
    pthread_mutex_lock(&all_queues_lock);
    for (int i = 0; i < LEVELS; i++) {
        destroy_queue(all_queues[i]);
    }
    pthread_mutex_unlock(&all_queues_lock);
    puts("Queues destroyed");

    return EXIT_SUCCESS;
}

/**
 * Spawns a car every 1..100 milliseconds Cars are given a
 * randomised license plate and directed to a random queue.
 * 
 * @param thread args
 * @return NULL upon completion
 */
void *spawn_cars(void *args) {

    while (!end_simulation) {
        pthread_mutex_lock(&rand_lock);
        /* spawn every 1..100 milliseconds */
        int pause_spawn = ((rand() % 100) + 1) * 1000000;
        int q_to_goto = rand() % ENTRANCES;
        pthread_mutex_unlock(&rand_lock);

        /* wait before spawning new car */
        sleep_for(0, pause_spawn);
        car_t *new_c = malloc(sizeof(car_t) * 1);
        
        /* assign random plate and goto random entrance */
        //random_plate(new_c);
        strcpy(new_c->plate, "621VWC");

        pthread_mutex_lock(&all_queues_lock);
        push_queue(all_queues[q_to_goto], new_c);
        pthread_mutex_unlock(&all_queues_lock);
    }
    return NULL;
}

/**
 * Generates a random license plate in the format of
 * 3 digits and 3 alphabet characters '111AAA'. Assigns
 * this string to the given car.
 * 
 * @param car to assign random license plate to
 */
void random_plate(car_t *c) {
    /* random plate to fill */
    char rand_plate[7];

    /* 3 random numbers */
    for (int i = 0; i < 3; i++) {
        pthread_mutex_lock(&rand_lock);
        char rand_number = "123456789"[rand() % 9];
        pthread_mutex_unlock(&rand_lock);
        rand_plate[i] = rand_number;
    }

    /* 3 random letters */
    for (int i = 3; i < 6; i++) {
        pthread_mutex_lock(&rand_lock);
        char rand_letter = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 26];
        pthread_mutex_unlock(&rand_lock);
        rand_plate[i] = rand_letter;
    }

    /* don't forget the null terminator */
    rand_plate[7] = '\0';

    /* assign to this car */
    strcpy(c->plate, rand_plate);
}

/**
 * {{{{{{{{{{{{{{{{{HANDLE ENTRANCE}}}}}}}}}}}}}}}}}
 * Big worker thread for each entrance, communicating
 * with the Manager software for verification, automation
 * and decision-making.
 * 
 * @param thread args to be deconstructed
 * @return NULL upon completion
 */
void *handle_entrance(void *args) {

    /* deconstruct args */
    en_args_t *a = (en_args_t *)args;
    int floor = a->number;
    void *shm = a->shared_memory;
    queue_t *q = a->queue; 

    /* locate associated shared memory data for this entrance */
    entrance_t *en = (entrance_t*)(shm + (sizeof(entrance_t) * floor));

    en->gate.status = 'C'; /* boom gate is always initialy closed */

    while (!end_simulation) {

        /* grab next car in line */
        if (q->head != NULL) {
            pthread_mutex_lock(&all_queues_lock);
            car_t *c = pop_queue(q);
            pthread_mutex_unlock(&all_queues_lock);


            /* 2 millisecond wait before triggering LPR */
            sleep_for(0, 2000000);

            /* lock LPR, read plate, unlock LPR, signal manager */
            pthread_mutex_lock(&en->sensor.lock);
            strcpy(en->sensor.plate, c->plate);
            pthread_mutex_unlock(&en->sensor.lock);
            pthread_cond_signal(&en->sensor.condition);
            
            
            /* lock sign, wait for manager to validate/update sign */
            pthread_mutex_lock(&en->sign.lock);

            while (en->sign.display == 0) {

                //puts("waiting for manager to validate plate");
                // prevents race condition
                pthread_cond_wait(&en->sign.condition, &en->sign.lock);

            }
    
    
            if (en->sign.display != 'F' && en->sign.display != 'X') {
                printf("sign: %c\n", en->sign.display);

    
            }
    
    
    
            if (en->sign.display == 'X' || en->sign.display == 'F') {
               // puts("Car NOT authorised! now leaving...");
                //printf("plate: %s\n", c->plate);

                
                free(c); /* car leaves simulation */
            } else {
                //puts("Car authorised! now entering...");
                //printf("plate: %s\n", c->plate);
                
                /* assign floor to car */
                c->floor = (int)en->sign.display - '0';
                
                pthread_mutex_lock(&en->gate.lock);

                while (en->gate.status != 'O' && en->gate.status != 'R') {
                    pthread_cond_wait(&en->gate.condition, &en->gate.lock);
                }
    
                /* if raising, wait to fully raised */
                if (en->gate.status == 'R') {
                    //puts("raising gate");
                    sleep_for(0, 10000000); /* 10ms */
                    en->gate.status = 'O';
                }
                
                /* send car through before allowing the gate to close */
                free(c);
                pthread_mutex_unlock(&en->gate.lock);
                pthread_cond_signal(&en->gate.condition);
            }
            
            /* reset the sign, and ensure the boom gate is closed */
            pthread_mutex_lock(&en->gate.lock);
            while (en->gate.status != 'C' && en->gate.status != 'L') {
                pthread_cond_wait(&en->gate.condition, &en->gate.lock);
            }

            /* if lowering, wait till fully closed */
            if (en->gate.status == 'L') {
                //puts("lowering gate");
                sleep_for(0, 10000000); /* 10ms */
                en->gate.status = 'C';
            }
            pthread_mutex_unlock(&en->gate.lock);
            pthread_cond_signal(&en->gate.condition);

            en->sign.display = 0;
            pthread_mutex_unlock(&en->sign.lock);
            
        }
    }
    free(args);
    return NULL;
}

/* 
gcc -o ../SIMULATOR simulator.c parking.c queue.c sleep.c -Wall -lpthread -lrt
*/