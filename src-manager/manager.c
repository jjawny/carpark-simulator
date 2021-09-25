/*******************************************************
 * @file    manager.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Main file for the manager software. Automates
 *          aspects of running a carpark. All of the manager's
 *          header files link back here.
 ******************************************************/
#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <string.h>     /* for string stuff... */
#include <stdbool.h>    /* for bool stuff... */
#include <pthread.h>    /* for threads */
#include <ctype.h>      /* for isalpha, isdigit... */
#include <fcntl.h>      /* for file modes like O_RDWR */
#include <sys/mman.h>   /* for mapping shared like MAP_SHARED */

/* header APIs + read config file */
#include "parking-types.h"
#include "plates-hash-table.h"
#include "../config.h"

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920 /* bytes */
#define TABLE_SIZE 100 /* buckets for authorised license plates */

int main(int argc, char **argv) {
    /* READ IN AUTHORISED PLATES FILE INTO */
    htab_t *plates_ht = new_hashtable(TABLE_SIZE);
    
    FILE *fp = fopen("plates.txt", "r");
    if (fp == NULL) {
        perror("Athorised license plates file");
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

        /* check if line is in correct license plate format */
        /* first 3 characters are digits */
        for (int i = 0; i < (PLATE_SIZE / 2); i++) {
            if (isdigit(i) != 0) {
                printf("%s is an invalid plate\n", line);
                is_valid = false;
            }
        }

        /* last 3 characters are digits */
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




    /* locate the shared memory */
    int shm_fd;
    char *shm;

    if ((shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0)) < 0) {
        perror("Opening shared memory");
        exit(1);
    }

    /* attach the shared memory segment to this data space */
    if ((shm = mmap(0, SHARED_MEM_SIZE, PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (char *)-1) {
        perror("Mapping shared memory");
        exit(1);
    }

    

    puts("attempting to access shared mem");
    printf("found entrance 1's sensor's plate:\t%s\n", (char*)(shm + 88));
    entrance_t *en1 = (entrance_t*)(shm + 0);
    printf("found en1's LPR plate thru arrows\t%s\n", en1->sensor.plate);




    /* destroy license plates' hash table */
    hashtable_destroy(plates_ht);
    return EXIT_SUCCESS;
}


/*

The roles of the manager:
● Monitor the status of the LPR sensors and keep 
track of where each car is in the car park

● Tell the boom gates when to open and when to close 
(the boom gates are a simple piece of hardware that can 
only be told to open or close, so the job of automatically
closing the boom gates after they have been open for a little while is up to the
manager)

● Control what is displayed on the information signs at each entrance

● As the manager knows where each car is, it is the manager’s job to ensure that there
is room in the car park before allowing new vehicles in (number of cars < number of
levels * the number of cars per level). The manager also needs to keep track of how
full the individual levels are and direct new cars to a level that is not fully occupied

● Keep track of how long each car has been in the parking lot and produce a bill once
the car leaves.

● Display the current status of the parking lot on a frequently-updating screen, showing
how full each level is, the current status of the boom gates, signs, temperature
sensors and alarms, as well as how much revenue the car park has brought in so far

*/


// gcc -o ../MANAGER manager.c plates-hash-table.c -Wall -lrt
