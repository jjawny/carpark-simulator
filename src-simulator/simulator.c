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

bool end_simulation = false;
queue_t *all_queues[ENTRANCES];

pthread_mutex_t end_simulation_lock; /* lock for simulation finished flag */
pthread_mutex_t all_queues_lock; /* lock for list of queues as this is global */
pthread_mutex_t rand_lock; /* lock for rand calls as seed is global */

/* entrance thread args */
typedef struct en_args_t {
    int number;
    void *shared_memory;
    queue_t *queue;
} en_args_t;

/* function prototypes */
void random_plate(car_t *c);
void *handle_entrance(void *args);
void *spawn_cars(void *args);

int main (int argc, char **argv) {
    /* random seed...
    - set to 1 for deterministic behaviour (debugging)
    - set to current time for true randomness */
    srand(time(NULL));

    /* create shared mem */
    /* shared mem will be unmapped at the end of main */
    void *shm = create_shared_memory(SHARED_MEM_NAME, SHARED_MEM_SIZE);
    init_shared_memory(shm, ENTRANCES, EXITS, LEVELS);
     
    /* create n QUEUES for entrances */
    /* all queues will be destroyed at the end of main */
    pthread_mutex_lock(&all_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        queue_t *new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        all_queues[i] = new_q;
    }
    pthread_mutex_unlock(&all_queues_lock);

    /* ENTRANCE THREADS */
    pthread_t en_threads[ENTRANCES];
    
    pthread_mutex_lock(&all_queues_lock);
    for (int i = 0; i < ENTRANCES; i++) {
        // args will be freed within relevant thread
        en_args_t *args = malloc(sizeof(en_args_t) * 1);
        args->number = i;
        args->shared_memory = shm;
        args->queue = all_queues[i];

        pthread_create(&en_threads[i], NULL, handle_entrance, (void *)args);
    }
    pthread_mutex_unlock(&all_queues_lock);

    /* START SPAWNING CARS THREAD*/
    /* cars will be freed at the end of their lifecycle thread */
    pthread_t spawn_cars_thread;
    pthread_create(&spawn_cars_thread, NULL, spawn_cars, NULL);

    /* simulation runs for n seconds, then alerts all 
    threads to finish their current cycle before cleanup */
    pthread_mutex_lock(&end_simulation_lock);
    sleep(DURATION);
    end_simulation = true;
    pthread_mutex_unlock(&end_simulation_lock);

    puts("Simulation ended, now cleaning up...");
    
    /* join ALL threads before cleanup */
    for (int i = 0; i < ENTRANCES; i++) {
        pthread_join(en_threads[i], NULL);
    }

    /* CLEANUP */

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

void *spawn_cars(void *args) {
    while (!end_simulation) {
        pthread_mutex_lock(&rand_lock);
        /* 1000000..100000000 nanoseconds = 1..100 milliseconds */
        int pause_spawn = ((rand() % 100) + 1) * 1000000;
        int q_to_goto = rand() % 5;
        pthread_mutex_unlock(&rand_lock);

        /* wait before spawning new car */
        sleep_for(0, pause_spawn);
        car_t *new_c = malloc(sizeof(car_t) * 1);
        
        /* assign random plate and goto random entrance */
        random_plate(new_c);
        pthread_mutex_lock(&all_queues_lock);
        push_queue(all_queues[q_to_goto], new_c);
        pthread_mutex_unlock(&all_queues_lock);
    }
    return NULL;
}

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

void *handle_entrance(void *args) {
    
    /* deconstruct args */
    en_args_t *a = (en_args_t *)args;
    int floor = a->number;
    void *shm = a->shared_memory;
    queue_t *q = a->queue;
    
    /* locate associated shared memory data for this entrance */
    entrance_t *en = (entrance_t*)(shm + (sizeof(entrance_t) * floor));

    while (!end_simulation) {
        
        /* grab next in line */
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
            


            pthread_mutex_lock(&en->sign.lock);
            /* wait until LPR hardware reads in a number plate */
            while (en->sign.display == 0) {
                puts("waiting for manager to validate plate");

                pthread_cond_wait(&en->sign.condition, &en->sign.lock);

            }
            puts("validated!");

            if (en->sign.display == 'X' || en->sign.display == 'F') {
                free(c); /* car leaves simulation */
                puts("car not authorised, now leaving");
            } else {
                free(c);
                puts("car YES, now entering");
            }
            

            /* after dealing with car, clear the sign */
            en->sign.display = 0;
            pthread_mutex_unlock(&en->sign.lock);
        }
    }




    

    
    free(args);
    
    return NULL;
}

// gcc -o ../SIMULATOR simulator.c parking.c queue.c sleep.c -Wall -lpthread -lrt
