/************************************************
 * @file    sleep.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for sleep.h
 ***********************************************/
#include <stdio.h>   /* for IO operations */
#include <time.h>    /* for time operations */

#include "sleep.h"   /* corresponding header */

void sleep_for_millis(int ms) {
   /* slow down time for debugging, 1 = no change */
   long int scale = 1;

   struct timespec remaining, requested = {(ms / 1000) * scale, ((ms % 1000) * 1000000) * scale};
   nanosleep(&requested, &remaining);
}