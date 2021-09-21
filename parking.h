#pragma once

#include <pthread.h>
#include <stdint.h>

typedef struct LPR_t {
    pthread_mutex_t lock;
    pthread_cond_t condition;
    char plate[7]; /* 6 chars +1 for string null terminator */
    char padding[1]; /* as we +1 above, we only need to +1 for padding, not +2 */
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


/* might be redundant */
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
    int16_t temp_sensor; /* 2 bytes - signed 16 bit int */
    char alarm; /* 1 byte - either a 0 or a 1 */
    char padding[5];
} level_t;