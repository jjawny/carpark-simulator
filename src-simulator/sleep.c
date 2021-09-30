/************************************************
 * @file    sleep.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for sleep.h
 ***********************************************/
#include <time.h>    /* for time operations */
#include "sleep.h"   /* corresponding header */

void sleep_for_millis(int ms) {
   /* slow down time for debugging, 1 = no change */
   long int scale = 3;

   struct timespec remaining, requested = {(ms / 1000) * scale, ((ms % 1000) * 1000000) * scale};
   nanosleep(&requested, &remaining);
}