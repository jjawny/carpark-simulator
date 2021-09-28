#include <string.h>
#include <pthread.h>
#include <stdlib.h>

#include "sim-common.h"
#include "queue.h"
#include "sleep.h"

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
        int pause_spawn = ((rand() % 100) + 1) * 1000000;
        int q_to_goto = rand() % ENTRANCES;
        pthread_mutex_unlock(&rand_lock);

        /* wait 1..100 milliseconds before spawning new car */
        sleep_for(0, pause_spawn);
        car_t *new_c = malloc(sizeof(car_t) * 1);
        
        /* assign random plate and goto random entrance */
        //random_plate(new_c);
        strcpy(new_c->plate, "621VWC");

        pthread_mutex_lock(&en_queues_lock);
        push_queue(en_queues[q_to_goto], new_c);
        pthread_mutex_unlock(&en_queues_lock);
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