/************************************************
 * @file    fire-common.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Common function among Fire Alarm's seperate source files.
 ***********************************************/

/* for timespec type and nanosleep - violates MISRA C but unavoidable due to 
needing to control sensitive timings in milliseconds, minimised by only 1 function scope */
#include <time.h>

#include "fire-common.h"  /* corresponding header */

void sleep_for_millis(int ms) {
   struct timespec remaining, requested = {(ms / 1000) * SLOW, ((ms % 1000) * 1000000) * SLOW};
   nanosleep(&requested, &remaining);
}