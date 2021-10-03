/************************************************
 * @file    parking.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for setup, initialisation, and destruction
 * of a shared memory object. Only the Simulator may create 
 * the shared memory object, where the Manager and Fire Alarm System 
 * may open and map the memory into their own data space for use.
 * 
 * Formulas for locating segments of the PARKING shared memory, 
 * where 'i' increments from 0 to less-than the number of 
 * ENTRANCES/EXITS/LEVELS respectively.
 * 
 * entrances: (sizeof(en) * i)
 * exits:     (sizeof(en) * total en) + (sizeof(ex) * i)
 * levels:    (sizeof(en) * total en) + (sizeof(ex) * total ex) + (sizeof(lvl) * i)
 ***********************************************/
#pragma once

#include <pthread.h>    /* for mutexes/conditions */
#include <stdint.h>     /* for 16 bit int type */

/* TYPES THAT WILL BE NESTED */
typedef struct LPR_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char plate[7];      /* 6 chars +1 for string null terminator */
    char padding[1];    /* as we +1 above, we only need to +1 for padding, not +2 */
} LPR_t;

typedef struct boom_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char status;        /* C,O,R,L - Closed, Open, Raising, Lowering */
    char padding[7];
} boom_t;

typedef struct info_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char display;       /* X,F,0..n - Not authorised, Full, Floor number*/
    char padding[7];
} info_t;

/* PARENT TYPES */
typedef struct entrance_t {
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
 * @brief Create a shared memory object or overwrites an older
 * copy with the same name if already exists.
 * 
 * @param name - name of the shared memory
 * @param size - size of the shared memory
 * @return void* - pointer to first byte of the shared memory
 */
void *create_shared_memory(char *name, size_t size);

/**
 * @brief Initialise the shared memory object. Filling the bytes up with
 * custom types for entrances, exists, levels, LPR sensors, boom gates,
 * mutex locks etc. As each item is added, an offset increases to ensure 
 * no item overwrites another. All mutexes and condition variables are 
 * setup for inter-process communication. 
 * 
 * @param shm - pointer to first byte of the shared memory
 * @param entrances - no. of entrances
 * @param exits - no. of exits
 * @param levels - no. of levels
 */
void init_shared_memory(void *shm, int entrances, int exits, int levels);

/**
 * @brief Unmaps and unlinks the shared memory object.
 * 
 * @param shm - pointer to first byte of the shared memory
 * @param size - size of the shared memory
 * @param name - name of the shared memory
 */
void destroy_shared_memory(void *shm, size_t size, char *name);