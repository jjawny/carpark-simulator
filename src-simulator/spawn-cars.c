/************************************************
 * @file    spawn-cars.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for spawn-cars.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <string.h>     /* for string operations */
#include <pthread.h>    /* for thread operations */
#include <stdlib.h>     /* for misc like rand */
#include <ctype.h>      /* for isdigit/isalpha */

#include "spawn-cars.h" /* corresponding header */
#include "sim-common.h" /* for flag & rand lock */
#include "queue.h"      /* for queue operations */
#include "sleep.h"      /* for custom millisecond sleep */

/* function prototypes */
void random_plate(car_t *c);
void random_chance(car_t *c, float chance, item_t **pool, int total);
bool validate_plate(char *p);

void *spawn_cars(void *args) {

    /* -----------------------------------------------
     *                  DECONSTRUCT ARGS
     * -------------------------------------------- */
    args_t *a = (args_t *)args;

    /* -----------------------------------------------
     *     READ PLATES.TXT FILE INTO DYNAMIC ARRAY
     * -------------------------------------------- */
    size_t pool_size = 100;
    int added = 0; /* also the index */

    item_t **pool = malloc(sizeof(item_t *) * pool_size);
    if (pool == NULL) {
        perror("malloc pool in spawn-cars thread");
        exit(1);
    }

    FILE *fp = fopen("plates.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }

    char line[1000]; /* buffer to ensure whole line is read */

    while (fgets(line, sizeof(line), fp) != NULL) {
        
        /* scan line for first occurrence of the newline
         * and replace with a null terminator */
        line[strcspn(line, "\n")] = 0;
        
        if (validate_plate(line)) {
            item_t *new_i = malloc(sizeof(item_t));
            strncpy(new_i->plate, line, 7);
            pool[added] = new_i;
            //printf("%s, %s, %s\n", line, new_i->plate, pool[added]->plate);
            added++;
        }

        /* if we've run out of memory, realloc the array */
        if (added >= (int)pool_size) {
            pool_size *= 2;

            pool = realloc(pool, pool_size * sizeof(item_t *));
            if (pool == NULL) {
                perror("realloc pool in spawn-cars thread");
                exit(1);
            }
        }
        
    }
    //printf("%d plates added\n", added);
    fclose(fp);

    /* -----------------------------------------------
     *        LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while (!end_simulation) {
        /* lock rand call for random entrance and milliseconds wait */
        pthread_mutex_lock(&rand_lock);
        int pause_spawn = ((rand() % 100) + 1); /* 1..100 */
        int q_to_goto = rand() % a->ENS;
        pthread_mutex_unlock(&rand_lock);

        /* wait 1..100 milliseconds before spawning a new car */
        sleep_for_millis(pause_spawn);
        car_t *new_c = malloc(sizeof(car_t) * 1);
        
        /* -----------------------------------------------
         *          TOGGLE FOR DEMO / DEBUGGING
         *                  FIXED PLATE
         *              CONTROLLED RANDOMNESS
         * -------------------------------------------- */
        //strcpy(new_c->plate, "206WHS");
        random_chance(new_c, a->CH, pool, added);

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

    /* free each individual item before the array itself */
    for (int i = 0; i < added; i++) free(pool[i]);
    free(pool);

    free(a); /* free args */
    return NULL;
}

bool validate_plate(char *p) {
    /* check if plate is correct length */
    if (strlen(p) != 6) return false;
    
    /* slice string in half - first 3, last 3 */
    char first[4];
    char last[4];
    strncpy(first, p, 3);
    strncpy(last, p + 3, 3);
    first[3] = '\0';
    last[3] = '\0';

    /* for debugging... */
    //printf("FIRST3\t%s\n", first);
    //printf("LAST3\t%s\n", last);

    /* check if plate is correct format: 111AAA */
    for (int i = 0; i < 3; i++) {
        if (!(isdigit(first[i]) && isalpha(last[i]))) return false;
    }
    return true;
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

void random_chance(car_t *c, float chance, item_t **pool, int total) {
    /* check bounds and default to 50% chance if out-of-bounds */
    if (chance > 1 || chance < 0) chance = (float)0.50;

    float n = 0;
    pthread_mutex_lock(&rand_lock);
    n = (float)((rand() % 100) + 1) / 100; /* 0..99 +1 for 1..100 then /100 for 0.00..1.00 */
    pthread_mutex_unlock(&rand_lock);

    /* assign to this car */
    if (n < chance) {
        int index = 0;
        pthread_mutex_lock(&rand_lock);
        index = rand() % total;
        pthread_mutex_unlock(&rand_lock);
        /* since there are a finite no. of authorised cars
         * versus millions non-authorised, we will only assign
         * a non-authorised plate n% of the time */
        strcpy(c->plate, pool[index]->plate);
    } else {
        /* assign truly random plate */
        random_plate(c);
    }
}