/************************************************
 * @file    sleep.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for sleep.h
 ***********************************************/
#include <time.h>    /* for timespec and nanosleep */

#include "sleep.h"   /* corresponding header */
#include "sim-common.h" /* for slow motion value */

void sleep_for_millis(int ms) {
   struct timespec remaining, requested = {(ms / 1000) * SLOW, ((ms % 1000) * 1000000) * SLOW};
   nanosleep(&requested, &remaining);
}