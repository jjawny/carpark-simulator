
#include "handle-entrance.h"
#include "sleep.h"
#include "parking.h"
#include "queue.h"
#include "sim-common.h"


/**
 * {{{{{{{{{{{{{{{{{HANDLE ENTRANCE}}}}}}}}}}}}}}}}}
 * Big worker thread for each entrance, communicating
 * with the Manager software for verification, automation
 * and decision-making.
 * 
 * @param thread args to be deconstructed
 * @return NULL upon completion
 */
void *handle_entrance(void *args) {

    /* deconstruct args */
    en_args_t *a = (en_args_t *)args;
    int floor = a->number;
    char *shm = a->shared_memory;
    queue_t *q = a->queue; 

    /* locate associated shared memory data for this entrance */
    entrance_t *en = (entrance_t*)(shm + (sizeof(entrance_t) * floor));

    en->gate.status = 'C'; /* boom gate is always initialy closed */

    while (!end_simulation) {

        /* grab next car in line */
        if (q->head != NULL) {
            pthread_mutex_lock(&en_queues_lock);
            car_t *c = pop_queue(q);
            pthread_mutex_unlock(&en_queues_lock);


            /* 2 millisecond wait before triggering LPR */
            sleep_for(0, 2000000);

            /* lock LPR, read plate, unlock LPR, signal manager */
            pthread_mutex_lock(&en->sensor.lock);
            strcpy(en->sensor.plate, c->plate);
            pthread_mutex_unlock(&en->sensor.lock);
            pthread_cond_signal(&en->sensor.condition);
            
            
            /* lock sign, wait for manager to validate/update sign */
            pthread_mutex_lock(&en->sign.lock);

            while (en->sign.display == 0) {

                //puts("waiting for manager to validate plate");
                // prevents race condition
                pthread_cond_wait(&en->sign.condition, &en->sign.lock);

            }
    
    
            if (en->sign.display != 'F' && en->sign.display != 'X') {
                printf("sign: %c\n", en->sign.display);

    
            }
    
    
    
            if (en->sign.display == 'X' || en->sign.display == 'F') {
               // puts("Car NOT authorised! now leaving...");
                //printf("plate: %s\n", c->plate);

                
                free(c); /* car leaves simulation */
            } else {
                //puts("Car authorised! now entering...");
                //printf("plate: %s\n", c->plate);
                
                /* assign floor to car */
                c->floor = (int)en->sign.display - '0';
                
                pthread_mutex_lock(&en->gate.lock);

                while (en->gate.status != 'O' && en->gate.status != 'R') {
                    pthread_cond_wait(&en->gate.condition, &en->gate.lock);
                }
    
                /* if raising, wait to fully raised */
                if (en->gate.status == 'R') {
                    //puts("raising gate");
                    sleep_for(0, 10000000); /* 10ms */
                    en->gate.status = 'O';
                }
                
                /* send car through before allowing the gate to close */
                free(c);
                pthread_mutex_unlock(&en->gate.lock);
                pthread_cond_signal(&en->gate.condition);
            }
            
            /* reset the sign, and ensure the boom gate is closed */
            pthread_mutex_lock(&en->gate.lock);
            while (en->gate.status != 'C' && en->gate.status != 'L') {
                pthread_cond_wait(&en->gate.condition, &en->gate.lock);
            }

            /* if lowering, wait till fully closed */
            if (en->gate.status == 'L') {
                //puts("lowering gate");
                sleep_for(0, 10000000); /* 10ms */
                en->gate.status = 'C';
            }
            pthread_mutex_unlock(&en->gate.lock);
            pthread_cond_signal(&en->gate.condition);

            en->sign.display = 0;
            pthread_mutex_unlock(&en->sign.lock);
            
        }
    }
    free(args);
    return NULL;
}
