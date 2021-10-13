/************************************************
 * @file    fire-gate.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for fire-gate.h
 ***********************************************/
#include <pthread.h> /* for mutex/condition types */
#include <unistd.h>  /* for misc like sleep */

#include "fire-common.h"    /* common among fire alarm sys */
#include "fire-gate.h"      /* corresponding header */

void *open_gate(void *args) {

    (void)args; /* supresses unused var warning - args param is unused but mandatory */
    int active = 0; /* 0 = no, 1 = yes */

    /* -----------------------------------------------
     *       LOOP WHILE SIMULATION HASN'T ENDED
     * -------------------------------------------- */
    while(!end_simulation) {
        active = 0; /* reset */

        /* Wait until the alarm is active then store in another 
        variable so we can unlock and let other threads see
        if the alarm is active */ 
        pthread_mutex_lock(&alarm_m);
        while (!alarm_active && !end_simulation) pthread_cond_wait(&alarm_c, &alarm_m);
        if (alarm_active) active = 1; 
        pthread_mutex_unlock(&alarm_m);

        /* -----------------------------------------------
         *   IF THERE IS A FIRE & SIM HASN'T ENDED...
         *   RAISE ALL ENTRACE/EXIT GATES
         * -------------------------------------------- */
        if (!end_simulation && active) {
            for (int i = 0; i < ENS; i++) {
                entrance_t *en = (entrance_t *)((char *)shm + (int)(sizeof(entrance_t) * i));

                pthread_mutex_lock(&en->gate.lock);
                if (en->gate.status == 'C') en->gate.status = 'R';
                pthread_mutex_unlock(&en->gate.lock);
                pthread_cond_broadcast(&en->gate.condition);
            }

            for (int i = 0; i < EXS; i++) {
                exit_t *ex = (exit_t *)((char *)shm + (int)((sizeof(entrance_t) * ENS) + (sizeof(exit_t) * i)));

                pthread_mutex_lock(&ex->gate.lock);
                if (ex->gate.status == 'C') ex->gate.status = 'R';
                pthread_mutex_unlock(&ex->gate.lock);
                pthread_cond_broadcast(&ex->gate.condition);
            }
        }
        /* unlike the EVACUATE sign, we will only need to open once,
        (but check now and then if the gate is closed to raise again) 
        otherwise we'll be looping forever - this 5s pause saves resources */
        sleep(5);

        /* as we must let other threads access the "alarm_active", we will have to loop back up,
        lock-read-unlock the "alarm_active" again then display EVACUATE if there is still a fire */
    }
    return NULL;
}