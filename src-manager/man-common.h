/************************************************
 * @file    man-common.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Common items among Manager's seperate source files
 * such as Shared memory types so the Manager only needs to
 * locate the first byte of an entrance/exit/level in order to 
 * access all of its attributes (with arrow notation). Also 
 * includes global capacity counts with lock/condvar.
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

#include <pthread.h>    /* for threads */
#include <stdint.h>     /* for 16-bit integer type */

#include "plates-hash-table.h" /* for # table type */

/* ALL GLOBALS 
Rather than constantly passing pointers around 
(essentially making them global already) let them 
be global but restrict access using mutex locks */

extern void *shm;                       /* first byte of shared mem */
extern int *curr_capacity;              /* array of capacity per level */
extern pthread_mutex_t curr_capacity_lock;
extern pthread_cond_t curr_capacity_cond;
extern _Atomic int revenue;             /* total $$$ */
extern _Atomic int total_cars_entered;  /* total cars in/out */
extern htab_t *auth_ht;                 /* authorised cars # table */
extern htab_t *bill_ht;                 /* billing # table */
extern pthread_mutex_t auth_ht_lock;
extern pthread_mutex_t bill_ht_lock;
extern pthread_cond_t auth_ht_cond;
extern pthread_cond_t bill_ht_cond;

/* entrance/level/exit thread args */
typedef struct args_t {
    int id;
} args_t;

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