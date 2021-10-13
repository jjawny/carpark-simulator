/************************************************
 * @file    fire-evac.c
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   Source code for fire-evac.h
 ***********************************************/
#include <pthread.h>    /* for mutex/condition types */
#include <string.h>     /* for string operations */

#include "fire-common.h"    /* common among fire alarm sys */
#include "fire-evac.h"      /* corresponding header */

void *evac_sign(void *args) {

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
         *   LOOP THRU EACH LETTER OF "EVACUATE" FOR
         *   EACH ENTRANCE SIGN EVERY 20ms
         * -------------------------------------------- */
        if (!end_simulation && active) {
            char *msg = "EVACUATE";

            for (int i = 0; i < (int)strlen(msg); i++){
                for (int e = 0; e < ENS; e++) {
                    entrance_t *en = (entrance_t *)((char *)shm + (int)(sizeof(entrance_t) * e));

                    pthread_mutex_lock(&en->sign.lock);
                    en->sign.display = msg[i];
                    pthread_mutex_unlock(&en->sign.lock);
                    pthread_cond_broadcast(&en->sign.condition);
                }
                sleep_for_millis(20);
            }
        }
        /* as we must let other threads access the "alarm_active", we will have to loop back up,
        lock-read-unlock the "alarm_active" again then display EVACUATE if there is still a fire */
    }
    return NULL;
}