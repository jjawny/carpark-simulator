/*******************************************************
 * @file    test
 * @brief   so far reads in a txt file of license plates, and does
 * more shit cool
 * @author  Johnny Madigan
 * @date    September 2021
 ******************************************************/
#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <string.h>     /* for string stuff... */
#include <stdbool.h>    /* for bool stuff... */

#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>

#include "plates-hash-table.h"
#include "parking.h"
#include "queue.h"



//gcc -o test test.c plates-hash-table.c parking.c queue.c -Wall -lpthread -lrt

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920    /* bytes */
#define TABLE_SIZE 100          /* buckets for license plates */
#define LEVELS 5                /* entrance, floor, exit */
#define CAPACITY 20             /* cars per level */

#define TOTAL_CARS 10 /* amount of cars to simulate */


int main (int argc, char **argv) {
    
    /* create shared mem */
    /* shared mem will be unmapped at the end of main */
    void *PARKING = create_shared_memory(SHARED_MEM_NAME, SHARED_MEM_SIZE);
    init_shared_memory(PARKING, LEVELS);
     
    //----------HASH TABLE BELOW------------
    htab_t *plates_ht = new_hashtable(TABLE_SIZE);
    
    FILE *fp = fopen("plates.txt", "r");
    if (fp == NULL) {
        perror("Unable to open athorised number plates file");
        exit(EXIT_FAILURE);
    }

    char line[1000]; /* ensure whole line is read */

    /* read and add to hash table line by line */
    while (fgets(line, sizeof(line), fp) != NULL) {


        /* scan line for first occurance of newline char 
        and replace with null terminator */
        line[strcspn(line, "\n")] = 0;

        bool is_valid = true;

        /* check line is legit size*/
        if (strlen(line) != PLATE_SIZE) {
            printf("%s is an invalid plate\n", line);
            is_valid = false;
        }

        /* check if line is correct format */
        for (int i = 0; i < (PLATE_SIZE / 2); i++) {
            if (isdigit(i) != 0) {
                printf("%s is an invalid plate\n", line);
                is_valid = false;

            }
        }

        for (int i = (PLATE_SIZE / 2); i < PLATE_SIZE; i ++) {
            if (isalpha(i) != 0) {
                printf("%s is an invalid plate\n", line);
                is_valid = false;
            }
        }

        if (is_valid) {
            
            hashtable_add(plates_ht, line);
        }
    }

    fclose(fp);
    //print_hashtable(plates_ht);
    //----------HASH TABLE ABOVE------------





    // SOMEWHERE HERE START THREADS FOR ENTRANCE/EXIT/LEVEL


    /* create n queues for entrances */
    /* queues will be freed at the end of main */
    queue_t *all_queues[LEVELS];
    for (int i = 0; i < LEVELS; i++) {
        queue_t *new_q = malloc(sizeof(queue_t) * 1);
        init_queue(new_q);
        all_queues[i] = new_q;
    }

    print_queue(all_queues[0]);



    /* create m cars to simulate */
    /* cars will be freed at the end of their lifecycle thread */
    for (int i = 0; i < 1; i++) {
        car_t *new_c = malloc(sizeof(car_t) * 1);
        int totally_random = 0;
        strcpy(new_c->plate, "YEET69");
        push_queue(all_queues[totally_random], new_c);
    }
    
    print_queue(all_queues[0]);

    /* for debugging, locating items in shared memory...
    pthread_mutex_t *LPR_lock = (pthread_mutex_t*)(PARKING + 0);
    printf("found entrance 1's sensor's plate:\t%s\n", (char*)(PARKING + 88));
    printf("found exit 1's sensor's plate:\t\t%s\n", (char*)(PARKING + 1528));
    printf("found floor 1's sensor's plate:\t\t%s\n", (char*)(PARKING + 2488));
    */

    /* for debugging, checking sizes of types...
    printf(" is %zu\n\n", sizeof(LPR_t));
    printf("Total is %zu\n\n", sizeof(boom_t));
    printf("Total is %zu\n\n", sizeof(info_t));
    printf("Total is %zu\n\n", sizeof(entrance_t));
    printf("Total is %zu\n\n", sizeof(exit_t));
    printf("Total is %zu\n\n", sizeof(level_t));
    */











    /* unmap shared memory */
    destroy_shared_memory(PARKING, SHARED_MEM_SIZE, SHARED_MEM_NAME);

    /* destroy license plates' hash table */
    hashtable_destroy(plates_ht);

    /* destroy all queues */
    for (int i = 0; i < LEVELS; i++) {
        destroy_queue(all_queues[i]);
    }

    return EXIT_SUCCESS;
}