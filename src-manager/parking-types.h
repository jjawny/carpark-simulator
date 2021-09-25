/*******************************************************
 * @file    parking-types.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Shared memory types so the Manager only needs to
 *          locate the first byte of an entrance/exit/level 
 *          in order to access all of its attributes 
 *          (with arrow notation)
 ******************************************************/
#pragma once

#include <pthread.h>    /* for threads */
#include <stdint.h>     /* for 16-bit integer type */

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