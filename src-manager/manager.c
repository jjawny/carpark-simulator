/************************************************
 * @file    manager.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Main file for the Manager software.
 *          Automates aspects of running a carpark.
 *          Threads branch out from here.
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <stdlib.h>     /* for dynamic memory */
#include <string.h>     /* for string operations */
#include <stdbool.h>    /* for bool type */
#include <pthread.h>    /* for threads */
#include <ctype.h>      /* for isalpha, isdigit... */
#include <fcntl.h>      /* for file modes like O_RDWR */
#include <sys/mman.h>   /* for mapping shared like MAP_SHARED */
#include <unistd.h>     /* for misc like sleep */
#include <stddef.h>     /* for offsetof */

/* header APIs + read config file */
#include "plates-hash-table.h"
#include "manage-entrance.h"
#include "manage-exit.h"
#include "manage-gate.h"
#include "display-status.h"
#include "man-common.h"
#include "../config.h"

#define SHARED_MEM_NAME "PARKING"
#define SHARED_MEM_SIZE 2920    /* size in bytes */
#define TABLE_SIZE 100          /* buckets for hash tables */

/* -----------------------------------------------
 *      INIT GLOBAL EXTERNS FROM man-common.h
 * -------------------------------------------- */
volatile _Atomic int end_simulation = 0;        /* 0 = no, 1 = yes */
volatile _Atomic int revenue = 0;               /* initially $0 */
volatile _Atomic int total_cars_entered = 0;    /* initially 0 cars */
volatile _Atomic int SLOW;                      /* slow down time by... */
volatile void *shm;                             /* first byte of shared memory */
int *curr_capacity;
pthread_mutex_t curr_capacity_lock;
pthread_cond_t curr_capacity_cond;
htab_t *auth_ht;
pthread_mutex_t auth_ht_lock;
pthread_cond_t auth_ht_cond;
htab_t *bill_ht;
pthread_mutex_t bill_ht_lock;
pthread_cond_t bill_ht_cond;

/* function prototypes */
void read_file(char *name, htab_t *table);
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
int main(void) {

    /* -----------------------------------------------
     *  CHECK BOUNDS for FOR USER INPUTS IN config.h
     * -----------------------------------------------
     * As the number of ENTRANCES/CAPACITY/CHANCE etc are
     * subject to human error, bounds must be checked.
     * Check bounds here ONCE for simplicity.
     */
    int ENS = ENTRANCES;
    int EXS = EXITS;
    int LVLS = LEVELS;
    int CAP = CAPACITY;
    int DU = DURATION;
    SLOW = SLOW_MOTION;

    puts("~Verifying ENTRANCES, EXITS, LEVELS are 1..5 inclusive...");
    if (ENTRANCES < 1 || ENTRANCES > 5) {
        ENS = 5;
        printf("\tENTRANCES out of bounds. Falling back to defaults (5)\n");
    }
    
    if (EXITS < 1 || EXITS > 5) {
        EXS = 5;
        printf("\tEXITS out of bounds. Falling back to defaults (5)\n");
    }
    
    if (LEVELS < 1 || LEVELS > 5) {
        LVLS = 5;
        printf("\tLEVELS out of bounds. Falling back to defaults (5)\n");
    }

    puts("~Verifying CAPACITY is greater than 0...");
    if (CAPACITY < 1) {
        CAP = 20;
        printf("\tCAPACITY out of bounds. Falling back to defaults (20)\n");
    }

    puts("~Verifying DURATION is greater than 0...");
    if (DURATION < 1) {
        DU = 60;
        printf("\tDURATION out of bounds. Falling back to defaults (1 minute)\n");
    }

    puts("~Verifying SLOW MOTION is at least 1...");
    if (SLOW_MOTION < 1) {
        SLOW = 1;
        printf("\tSLOW MOTION out of bounds. Falling back to defaults (1)\n");
    }

    /* Allocate dynamic memory to array to keep track of each level's current capacity,
     * all capacities are initially 0 meaning no cars are assigned */
    curr_capacity = calloc(LVLS, sizeof(int));

    /* -----------------------------------------------
     *              CREATE NEW # TABLES...
     *              FOR AUTHORISING
     *              FOR BILLING
     * -------------------------------------------- */
    auth_ht = new_hashtable(TABLE_SIZE);
    bill_ht = new_hashtable(TABLE_SIZE);

    /* -----------------------------------------------
     *      READ AUTHORISED LICENSE PLATES FILE
     * -------------------------------------------- */
    puts("Reading plates.txt");
    read_file("plates.txt", auth_ht);

    /* -----------------------------------------------
     *          LOCATE THE SHARED MEMORY OBJECT
     * -------------------------------------------- */
    puts("Locating Simulator's shared memory object");
    int shm_fd;

    if ((shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0)) < 0) {
        perror("Opening shared memory");
        exit(1);
    }

    /* attach the shared memory to this data space */
    if ((shm = mmap(0, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (char *)-1) {
        perror("Mapping shared memory");
        exit(1);
    }

    /* -----------------------------------------------
     *      START ENTRANCE, EXIT, & STATUS THREADS
     * -------------------------------------------- */
    puts("Starting entrance, exit, gate, and display status threads");
    pthread_t en_threads[ENS];
    pthread_t en_gates[ENS];
    pthread_t ex_threads[EXS];
    pthread_t ex_gates[EXS];
    pthread_t status_thread;
    int addr = 0;

    args_t *a;

    for (int i = 0; i < ENS; i++) {
        /* set up args - will be freed within their thread */
        a = malloc(sizeof(args_t) * 1);
        
        a->id = i;
        a->addr = (int)(sizeof(entrance_t) * i);
        a->ENS = ENS;
        a->EXS = EXS;
        a->LVLS = LVLS;
        a->CAP = CAP;

        pthread_create(&en_threads[i], NULL, manage_entrance, (void *)a);
        pthread_create(&en_gates[i], NULL, manage_en_gate, (void *)a);
    }

    for (int i = 0; i < EXS; i++) {
        /* set up args - will be freed within their thread */
        a = malloc(sizeof(args_t) * 1);
        
        a->id = i;
        a->addr = (int)((sizeof(entrance_t) * ENS) + (sizeof(exit_t) * i));
        a->ENS = ENS;
        a->EXS = EXS;
        a->LVLS = LVLS;
        a->CAP = CAP;

        pthread_create(&ex_threads[i], NULL, manage_exit, (void *)a);
        pthread_create(&ex_gates[i], NULL, manage_ex_gate, (void *)a);
    }

    /* set up args - will be freed within their thread */
    a = malloc(sizeof(args_t) * 1);

    a->id = 0;
    a->addr = 0;
    a->ENS = ENS;
    a->EXS = EXS;
    a->LVLS = LVLS;
    a->CAP = CAP;

    pthread_create(&status_thread, NULL, display, (void *)a);

    /* -----------------------------------------------
     *          ALERT ALL THREADS TO FINISH
     * -------------------------------------------- */
    sleep(DU);
    end_simulation = 1;
    puts("~Manager ending, now cleaning up...");

    /* -----------------------------------------------
     *                      CLEAN UP
     * -----------------------------------------------
     * broadcast all LPRs to wake up entrances/exits/gate threads so,
     * they can exit gracefully
     */
    for (int i = 0; i < ENS; i++) {
        addr = (int)(sizeof(entrance_t) * i);
        entrance_t *en = (entrance_t*)((char *)shm + addr);
        pthread_cond_broadcast(&en->sensor.condition);
        pthread_cond_broadcast(&en->gate.condition);
    }

    for (int i = 0; i < EXS; i++) {
        addr = (int)((sizeof(entrance_t) * ENS) + (sizeof(exit_t) * i));
        exit_t *ex = (exit_t *)((char *)shm + addr);
        pthread_cond_broadcast(&ex->sensor.condition);
        pthread_cond_broadcast(&ex->gate.condition);
    }

    /* -----------------------------------------------
     *          JOIN ALL THREADS BEFORE EXIT
     * -------------------------------------------- */
    for (int i = 0; i < ENS; i++) {
        pthread_join(en_threads[i], NULL);
        pthread_join(en_gates[i], NULL);
    }
    for (int i = 0; i < EXS; i++) {
        pthread_join(ex_threads[i], NULL);
        pthread_join(ex_gates[i], NULL);
    }
    pthread_join(status_thread, NULL);
    puts("~All threads returned");

    /* -----------------------------------------------
     *          UNMAP SHARED MEMORY
     *          DESTROY # TABLES
     *          FREE CAPACITIES ARRAY
     * -------------------------------------------- */
    munmap((void *)shm, SHARED_MEM_SIZE);
    close(shm_fd);
    hashtable_destroy(auth_ht);
    hashtable_destroy(bill_ht);
    puts("~Hash tables destroyed");
    free(curr_capacity);
    free(a);
    puts("~Goodbye");
    puts("");
    return EXIT_SUCCESS;
}

/**
 * @brief Opens (or creates file if it does not exist) for reading.
 * Each line will be validated as a license plate. If valid, the plate
 * will be added to the # table of authorised cars.
 * 
 * @param name - name of the file to open/create
 * @param table - # table to add valid plates to
 */
void read_file(char *name, htab_t *table) {
    FILE *fp = fopen(name, "r");
    if (fp == NULL) {
        perror("fopen");
        exit(1);
    }
    
    char line[1000]; /* buffer to ensure whole line is read */

    while (fgets(line, sizeof(line), fp) != NULL) {

        /* scan line for first occurrence of the newline
         * and replace with a null terminator */
        line[strcspn(line, "\n")] = 0;
        
        /* if string is valid, add to authorised # table */
        if (validate_plate(line)) hashtable_add(table, line, 0);
    }

    /* for debugging... */
    //print_hashtable(auth_ht);
    fclose(fp);
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