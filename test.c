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

typedef struct car_t {
    char plate[7]; /* 6 chars +1 for string null terminator */
    int floor;
    long duration; /* milliseconds */
} car_t;


//gcc -o test test.c plates-hashtable.c parking.c -Wall -lpthread

#define SHARED_MEM_SIZE 2920    /* bytes */
#define TABLE_SIZE 100          /* buckets for license plates */
#define LEVELS 5                /* entrance, floor, exit */
#define CAPACITY 20             /* vehicles per level */



int main (int argc, char **argv) {
    
    /* create shared mem */
    void *PARKING = create_shared_memory(SHARED_MEM_SIZE);
    init_shared_memory(PARKING, LEVELS);
    
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


   for (int i = 0; i < 10; i++) {
       puts("lol");
   }




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




    /* unmap shared memory */
    if (munmap(PARKING, SHARED_MEM_SIZE) == -1) {
        perror("munmap failed for shared memory: PARKING");
    } else {
        puts("Shared memory unmapped!");
    }
    /* destroy license plates' hash table */
    if (hashtable_destroy(plates_ht)) {
        puts("Hash table destroyed!");
    }

    return EXIT_SUCCESS;
}