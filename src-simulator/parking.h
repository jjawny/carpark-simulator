/*******************************************************
 * @file    parking.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for setup, initialisation, or destruction of 
 *          a shared memory object.
 ******************************************************/
#pragma once

#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <string.h>     /* for string stuff... */
#include <stdbool.h>    /* for bool stuff... */
#include <pthread.h>    /* for the mutexes and conditions */
#include <sys/mman.h>   /* for mapping stuff... */
#include <fcntl.h>      /* for file modes like O_RDWR */
#include <unistd.h>     /* misc */

/**
 * FORMULAS for locating segments of the PARKING shared memory, where 'i'
 * increments from 0 to less-than the number of ENTRANCES/EXITS/LEVELS respectively.
 * 
 * for entrances: (sizeof(en) * i)
 * for exits:     (sizeof(en) * total en) + (sizeof(ex) * 1)
 * for levels:    (sizeof(en) * total en) + (sizeof(ex) * total ex) + (sizeof(lvl) * i)
 */

/* types that will be nested into entrances, exits, and levels */
typedef struct LPR_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char plate[7];      /* 6 chars +1 for string null terminator */
    char padding[1];    /* as we +1 above, we only need to +1 for padding, not +2 */
} LPR_t;

typedef struct boom_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char status;
    char padding[7];
} boom_t;

typedef struct info_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char display;
    char padding[7];
} info_t;

/* parent types */
typedef struct entrace_t {
    LPR_t sensor;
    boom_t gate;
    info_t sign;
} entrance_t;

typedef struct exit_t {
    LPR_t sensor;
    boom_t gate;
} exit_t;

typedef struct level_t {
    LPR_t sensor;
    int16_t temp_sensor;    /* 2 bytes - signed 16 bit int */
    char alarm;             /* 1 byte  - either a '0' or a '1' */
    char padding[5];
} level_t;

/**
 * Creates a shared memory object or overwrites an older copy with the
 * same name.
 * 
 * @param name of the shared memory
 * @param size of the shared memory
 * @return pointer to the first byte of the shared memory
 */
void *create_shared_memory(char *name, size_t size);

/**
 * Initialise the shared memory object. Filling the bytes up with
 * custom types for entrances, exists, levels, LPR sensors, boom gates,
 * mutex locks etc. As each item is added, an offset increases to ensure 
 * no item overwrites another. All mutexes and condition variables are 
 * setup for inter-process communication. 
 *  
 * @param pointer to first byte of the shared memory
 * @param no. of entrances
 * @param no. of exits
 * @param no. of levels
 */
void init_shared_memory(void *memory, int entrances, int exits, int levels);

/**
 * Unmaps and unlinks the shared memory object.
 * 
 * @param pointer to first byte of the shared memory
 * @param size of the shared memory
 * @param name of the shared memory
 */
void destroy_shared_memory(void *shm, size_t size, char *name);