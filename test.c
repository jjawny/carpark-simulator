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

#include "plates-hash-table.h"
#include "parking.h"

typedef struct car_t {
    char plate[7]; /* 6 chars +1 for string null terminator */
    int floor;
    long duration; /* milliseconds */
} car_t;



#define SHARED_MEM_SIZE 2920    /* bytes */
#define TABLE_SIZE 100          /* buckets for license plates */
#define LEVELS 5                /* entrance, floor, exit */
#define CAPACITY 20             /* vehicles per level */






void* create_shared_memory(size_t size) {
    int protection = PROT_READ | PROT_WRITE;

    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    return mmap(NULL, size, protection, visibility, -1, 0);
}




int main (int argc, char **argv) {

    void *PARKING = create_shared_memory(SHARED_MEM_SIZE);

    /* create n entrances */
    int offset = 0;

    // init shared memory func?
    for (int i = 0; i < 5; i++) {
        
        printf("adding level %d's entrance stuff\n", i + 1);
        LPR_t *sensor = malloc(sizeof(LPR_t));
        boom_t *gate = malloc(sizeof(boom_t));
        info_t *sign = malloc(sizeof(info_t));

        strcpy(sensor->plate, "123ABC");
        gate->status = 'G';
        memcpy(PARKING + offset, sensor, sizeof(LPR_t));
        offset += sizeof(LPR_t);
        memcpy(PARKING + offset, gate, sizeof(boom_t));
        offset += sizeof(boom_t);
        memcpy(PARKING + offset, sign, sizeof(info_t));
        offset += sizeof(info_t);
    }


    printf("found entrance 1's sensor's plate: %s\n", (char*)(PARKING + 88));
    printf("found entrance 2's sensor's plate: %s\n", (char*)(PARKING + 376));
    printf("found entrance 3's sensor's plate: %s\n", (char*)(PARKING + 664));
    printf("found entrance 4's sensor's plate: %s\n", (char*)(PARKING + 952));
    printf("found entrance 5's sensor's plate: %s\n", (char*)(PARKING + 1240));

    printf("found boom's status: %s\n", (char*)(PARKING + 184));
    




    /* for debugging, checking sizes of types...
    printf(" is %zu\n\n", sizeof(LPR_t));
    printf("Total is %zu\n\n", sizeof(boom_t));
    printf("Total is %zu\n\n", sizeof(info_t));
    printf("Total is %zu\n\n", sizeof(entrance_t));
    printf("Total is %zu\n\n", sizeof(exit_t));
    printf("Total is %zu\n\n", sizeof(level_t));
    */








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

    if (munmap(PARKING, SHARED_MEM_SIZE) == -1) {
        perror("munmap failed");
    }
    return EXIT_SUCCESS;
}