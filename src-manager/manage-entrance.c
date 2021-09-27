

#include "manage-entrance.h"
#include "parking-types.h"
#include "plates-hash-table.h"
#include "common.h"
#include "../config.h"

void *manage_entrance(void *args) {

    /* deconstruct args */
    en_args_t *a = (en_args_t *)args;
    int floor = a->number;
    char *shm = a->shared_memory;
    /* underlying type should be a char pointer so pointer arithmetc
    can happen below to locate the memory */

    /* locate associated shared memory data for this entrance */
    entrance_t *en = (entrance_t*)(shm + (sizeof(entrance_t) * floor));

    for (;;) {
        puts("looping");
        puts(" ");
        pthread_mutex_lock(&en->sensor.lock);
        /* wait until LPR hardware reads in a number plate */
        while (strcmp(en->sensor.plate, "") == 0) {
            //puts("waiting for simulation to read plate");

            pthread_cond_wait(&en->sensor.condition, &en->sensor.lock);
        }

        /* validate plate, locking the hash table as it is global */
        pthread_mutex_lock(&plates_ht_lock);
        bool authorised = hashtable_find(plates_ht, en->sensor.plate);

        pthread_mutex_unlock(&plates_ht_lock);

        /* after validating, clear the info sign */
        strcpy(en->sensor.plate, "");
        pthread_mutex_unlock(&en->sensor.lock);

        /* update infosign to direct the car to either enter or leave */
        pthread_mutex_lock(&en->sign.lock);
        //pthread_mutex_lock(&current_cap_lock);

        if (!authorised) {
            en->sign.display = 'X';

        } else if (current_cap >= (CAPACITY * LEVELS)) {
            en->sign.display = 'F';

        } else if (authorised) {
            current_cap++; /* reserve space as car is authorised */
            en->sign.display = '1'; /* goto floor 1 for now */
            //printf("%d\n", current_cap);
            pthread_mutex_lock(&en->gate.lock);
            /* if closed? raise */
            if (en->gate.status == 'C') {
                //puts("about to raise gate");
                en->gate.status = 'R';
            }
            pthread_mutex_unlock(&en->gate.lock);
            pthread_cond_signal(&en->gate.condition);
        }
        /* unlock current capacity, sign, and signal simulator */
        //pthread_mutex_unlock(&current_cap_lock);
        pthread_mutex_unlock(&en->sign.lock);
        pthread_cond_signal(&en->sign.condition);
        
        /* close gate if left open */
        pthread_mutex_lock(&en->gate.lock);
        //printf("current gate status is %c\n", en->gate.status);
        /* while gate status is still raising, wait - there WILL be a signal */
        while (en->gate.status == 'R') {
            //prevent race condition
            pthread_cond_wait(&en->gate.condition, &en->gate.lock);
        }
        /* if open? lower */
        if (en->gate.status == 'O') {
            //puts("about to lower gate");
            en->gate.status = 'L';
        }
        pthread_mutex_unlock(&en->gate.lock);
        pthread_cond_signal(&en->gate.condition);

        //puts("completed cycle");
    }

    return NULL;
}