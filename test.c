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

#include <pthread.h>

#include "plates-hash-table.h"





typedef struct LPR_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char plate[7]; /* +1 to accomodate for string's null terminator */
    char padding[1]; /* as we +1 above, we only need to +1 for padding, not +2 */
} LPR_t;

#define TABLE_SIZE 100

int main (int argc, char **argv) {



    printf("Total is %zu\n\n", sizeof(LPR_t));








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


    return EXIT_SUCCESS;
}