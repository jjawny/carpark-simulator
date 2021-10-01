/************************************************
 * @file    spawn-cars.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for spawn-cars.h
 ***********************************************/
#include <string.h>     /* for string operations */
#include <pthread.h>    /* for thread operations */
#include <stdlib.h>     /* for misc */

#include "sim-common.h" /* for locks and condition variables */
#include "../config.h"  /* for number of entrances */
#include "queue.h"      /* for queue functions */
#include "sleep.h"      /* for milliseconds sleep */
#include "spawn-cars.h" /* corresponding header */

void *spawn_cars(void *args) {

    while (!end_simulation) {
        /* lock rand call for random entrance and milliseconds wait */
        pthread_mutex_lock(&rand_lock);
        int pause_spawn = ((rand() % 100) + 1); /* 1..100 */
        int q_to_goto = rand() % ENTRANCES;
        pthread_mutex_unlock(&rand_lock);

        /* wait 1..100 milliseconds before spawning a new car */
        sleep_for_millis(pause_spawn);
        car_t *new_c = malloc(sizeof(car_t) * 1);
        
        /* assign random plate */
        //random_plate(new_c);
        strcpy(new_c->plate, "206WHS");
//printf("%s\n", new_c->plate);

        /* goto random entrance */
        pthread_mutex_lock(&en_queues_lock);
        push_queue(en_queues[q_to_goto], new_c);
        pthread_mutex_unlock(&en_queues_lock);
        pthread_cond_broadcast(&en_queues_cond); 
        /* after placing each car in a queue, broadcast to all entrance
        threads to check if their queue now has a car waiting, this means
        those threads can wait rather than constantly checking, preventing
        busy waiting */
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
    rand_plate[6] = '\0';

    /* assign to this car */
    strcpy(c->plate, rand_plate);
}