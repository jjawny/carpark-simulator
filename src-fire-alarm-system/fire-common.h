/************************************************
 * @file    fire-common.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Common items among Fire Alarm's seperate source files.
 ***********************************************/
#pragma once

#include <stdint.h>    /* for 16-bit integer type */
#include <pthread.h>   /* for mutex/condition types */

/* -----------------------------------------------
 *     ALL GLOBALS USED IN FIRE ALARM SOFTWARE
 * -----------------------------------------------
 * Rather than constantly passing pointers around
 * (essentially making them global already) let them
 * be global but restrict access using mutex locks
 * and the _Atomic keyword
 *
 * All defined in Main (fire-alarm.c)
 */
extern volatile _Atomic int ENS;
extern volatile _Atomic int EXS;
extern volatile _Atomic int LVLS;
extern volatile _Atomic int SLOW;

extern volatile void *shm;                     /* first byte of shared memory object */
extern volatile _Atomic int end_simulation;/* 0 = no, 1 = yes */
extern volatile _Atomic int alarm_active;  /* 0 = off, 1 = on */
extern pthread_mutex_t alarm_m;
extern pthread_cond_t alarm_c;


/* -----------------------------------------------
 *                 NESTED TYPES
 * -------------------------------------------- */
typedef struct LPR_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char plate[7];      /* 6 chars +1 for string null terminator */
    char padding[1];    /* as we +1 above, we only need to +1 for padding, not +2 */
} LPR_t;

typedef struct boom_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char status;        /* C,R,L,O - Closed, Raising, Lowering, Opened */
    char padding[7];
} boom_t;

typedef struct info_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char display;       /* X,F,number - Not authorised, Full, Assigned level*/
    char padding[7];
} info_t;

/* -----------------------------------------------
 *                 PARENT TYPES
 * -------------------------------------------- */
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
    volatile _Atomic int16_t temp_sensor;    /* 2 bytes - signed 16 bit int */
    volatile _Atomic char alarm;            /* 1 byte  - either a '0' or a '1' */
    char padding[5];
} level_t;

/**
 * @brief Sleeps for 'ms' milliseconds
 * 
 * @param ms - milliseconds to sleep
 */
void sleep_for_millis(int ms);