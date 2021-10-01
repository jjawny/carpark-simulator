/************************************************
 * @file    manager.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Main file for the Manager software.
 *          Automates aspects of running a carpark.
 *          Threads branch out from here.
 ***********************************************/
#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <string.h>     /* for string stuff... */
#include <stdbool.h>    /* for bool stuff... */
#include <pthread.h>    /* for threads */
#include <ctype.h>      /* for isalpha, isdigit... */
#include <fcntl.h>      /* for file modes like O_RDWR */
#include <sys/mman.h>   /* for mapping shared like MAP_SHARED */

#include <time.h>
#include <unistd.h>

/* header APIs + read config file */
#include "plates-hash-table.h"
#include "manage-entrance.h"
#include "man-common.h"
#include "../config.h"

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920    /* bytes */
#define TABLE_SIZE 100          /* buckets for hash tables */

/* init externs from man-common.h */
int *curr_capacity;
pthread_mutex_t curr_capacity_lock;
pthread_cond_t curr_capacity_cond;

/* init externs from "plates-hash-table.h" */
htab_t *auth_ht;
htab_t *bill_ht;
pthread_mutex_t auth_ht_lock;
pthread_mutex_t bill_ht_lock;
pthread_cond_t auth_ht_cond;
pthread_cond_t bill_ht_cond;

/* function prototypes */
bool validate_plate(char *plate);

/**
 * @brief   Entry point for the MANAGER software.
 *          Opens shared memory (exit if not found).
 *          Creates # tables before branches off to other 
 *          threads to manage entrances, levels and exits.
 * 
 * @param   argc - argument count, a standard param
 * @param   argv - arguments, a standard param
 * @return  int - indicating program's success or failure 
 */
int main(int argc, char **argv) {
    
    /* array to keep track of each lvl's current capacity */
    curr_capacity = calloc(LEVELS, sizeof(int));

    /* empty # tables to store car info to authorise/calculate billing */
    auth_ht = new_hashtable(TABLE_SIZE);
    bill_ht = new_hashtable(TABLE_SIZE);

    /* ---READ AUTHORISED LICENSE PLATES LINE-BY-LINE
    INTO HASH TABLE, VALIDATING EACH PLATE--- */
    puts("Opening plates.txt");
    FILE *fp = fopen("plates.txt", "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    puts("Reading plates.txt");

    char line[1000]; /* ensure whole line is read */

    while (fgets(line, sizeof(line), fp) != NULL) {

        /* scan line for first occurance of the newline 
        and replace with a null terminator */
        line[strcspn(line, "\n")] = 0;
        
        /* if string is valid, add to database */
        if (validate_plate(line)) {
            hashtable_add(auth_ht, line, 0);
        }
    }
    fclose(fp);

    /* for debugging...
    print_hashtable(auth_ht);
    */

    /* ---LOCATE THE SHARED MEMORY OBJECT--- */
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

    /* ---START MANAGE ENTRANCE THREADS--- */
    pthread_t en_threads[ENTRANCES];
    
    for (int i = 0; i < ENTRANCES; i++) {
        // args will be freed within relevant thread
        en_args_t *args = malloc(sizeof(en_args_t) * 1);
        args->number = i;
        args->shared_memory = shm;

        pthread_create(&en_threads[i], NULL, manage_entrance, (void *)args);
    }

    /* ---CLEANUP--- */
    /* ---JOIN ALL THREADS BEFORE EXIT--- */
    for (int i = 0; i < ENTRANCES; i++) {
        pthread_join(en_threads[i], NULL);
    }

    /* unmap shared mem, destroy # table, and free capacities array */
    munmap((void *)shm, SHARED_MEM_SIZE);
    hashtable_destroy(auth_ht);
    hashtable_destroy(bill_ht);
    free(curr_capacity);
    return EXIT_SUCCESS;
}

/**
 * @brief Validates license plate strings via their format (111AAA).
 * 
 * @param p - plate to validate
 * @return true - if valid
 * @return false - if illegal
 */
bool validate_plate(char *p) {
    /* check if plate is correct length */
    if (strlen(p) != 6) {
        return false;
    }

    /* slice string in half - first 3, last 3 */
    char first[4];
    char last[4];
    strncpy(first, p, 3);
    strncpy(last, p + 3, 3);
    first[3] = '\0';
    last[3] = '\0';

    /* for debugging...
    printf("FIRST3\t%s\n", first);
    printf("LAST3\t%s\n", last);
    */

    /* check if plate is correct format: 111AAA */
    for (int i = 0; i < 3; i++) {
        if (!(isdigit(first[i]) && isalpha(last[i])))  {
            return false;
        }
    }

    return true;
}