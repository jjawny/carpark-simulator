/************************************************
 * @file    fire-alarm.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Main file for the Fire Alarm software.
 *          Monitors the temperatures of carpark,
 *          takes action when there's a fire.
 *          Threads branch out from here.
 ***********************************************/
#include <pthread.h>   /* for multithreading */
#include <fcntl.h>     /* for file modes like O_RDWR */
#include <sys/mman.h>  /* for mapping shared like MAP_SHARED */
#include <unistd.h>    /* for misc like sleep */
#include <stdlib.h>     /* violates MISRA C but necessary to pass arg safely into threads */

#include "../config.h"      /* client's configurations */
#include "monitor-temp.h"   /* for detecting fire threads */
#include "fire-gate.h"      /* for opening boomgates threads */
#include "fire-evac.h"      /* for evacuation sign threads */
#include "fire-common.h"    /* common among fire alarm sys */

#define SHARED_MEM_NAME "PARKING" /* name of shared memory obj */
#define SHARED_MEM_SIZE 2920      /* size in bytes */

/* -----------------------------------------------
 *      INIT GLOBAL EXTERNS FROM fire-common.h
 * -----------------------------------------------
 * to avoid violating MISRA C RULE ??? when mallocing heap memory to a thread args struct,
 * allow the following values to be global
 */
volatile _Atomic int ENS = ENTRANCES;
volatile _Atomic int EXS = EXITS;
volatile _Atomic int LVLS = LEVELS;
volatile _Atomic int SLOW = SLOW_MOTION;

volatile void *shm;                     /* first byte of shared memory object */
volatile _Atomic int end_simulation = 0;/* 0 = no, 1 = yes */
volatile _Atomic int alarm_active = 0;  /* 0 = off, 1 = on */
pthread_mutex_t alarm_m = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t alarm_c = PTHREAD_COND_INITIALIZER;


int main(void) {
    /* -----------------------------------------------
     *  CHECK BOUNDS for FOR USER INPUTS IN config.h
     * -----------------------------------------------
     * As the number of ENTRANCES/CAPACITY/CHANCE etc are
     * subject to human error, bounds must be checked.
     * Check bounds here ONCE for simplicity and fall
     * back to defaults if out of bounds.
     */
    int DU = DURATION; /* within this scope only as it does not need to be global */

    if (ENTRANCES < 1 || ENTRANCES > 5) ENS = 5;
    if (EXITS < 1 || EXITS > 5) EXS = 5;
    if (LEVELS < 1 || LEVELS > 5) LVLS = 5;
    if (SLOW_MOTION < 1) SLOW = 1;
    if (DURATION < 1) DU = 60;

    /* -----------------------------------------------
     *       LOCATE THE SHARED MEMORY OBJECT
     * -------------------------------------------- */
    int exit; /* 0 = success, 1 = failure */
    int shm_fd = shm_open(SHARED_MEM_NAME, O_RDWR, 0);
    shm = (volatile void *) mmap(0, SHARED_MEM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);


    /* -----------------------------------------------
     * SPAWN THREADS ONLY IF SHM_OPEN & MMAP SUCCEEDED
     * -------------------------------------------- */
    if (!(shm_fd < 0) || !(shm == (char *)-1)) {
        /* if we reach here, when Main exits, it'll exit
        with success (0) */
        exit = 0; 

        pthread_t evac_thread;     
        pthread_t gate_thread;   
        pthread_t temp_threads[LVLS];

        for (int i = 0; i < LVLS; i++) {
            int *arg = malloc(sizeof(*arg)); /* violates misra c but passing the i value within a for loop causes unpredictable
            behaviour as the for loop can change the true value of i, meaning each thread fucks up */

            /* if malloc succeeded, the value of arg pointer is 'i' */
            if (arg != NULL) {
                *arg = i;
                pthread_create(&temp_threads[i], NULL, monitor_temp, (void *)arg);
            }
        }
        
        pthread_create(&evac_thread, NULL, evac_sign, NULL);
        pthread_create(&gate_thread, NULL, open_gate, NULL);

        /* -----------------------------------------------
         *          ALERT ALL THREADS TO FINISH
         * -------------------------------------------- */
        sleep(DU);
        end_simulation = 1;
        pthread_cond_broadcast(&alarm_c);

        /* -----------------------------------------------
         *          JOIN ALL THREADS BEFORE EXIT
         * -------------------------------------------- */
        for (int i = 0; i < LVLS; i++) {
            pthread_join(temp_threads[i], NULL);
        }
        pthread_join(evac_thread, NULL);
        pthread_join(gate_thread, NULL);

        

    } else {
        /* if we reach here, when Main exits, it'll exit
        with failure (1), we can only have 1 point of
        exit for each function otherwise we violate
        MISRA C rule 15.5 */
        exit = 1;
    }

    /* -----------------------------------------------
     *                     CLEAN UP
     * --------------------------------------------- */
    if (munmap((void *)shm, SHARED_MEM_SIZE) == -1) exit = 1;
    close(shm_fd);
    return exit;
}