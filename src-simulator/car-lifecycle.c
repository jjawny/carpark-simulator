
#include "queue.h"
#include <pthread.h>

void *car_lifecycle(void *car) {
    car_t *c = (car_t *)car;

    puts("I'M ALIVE");

    printf("i %s have been told to goto floor: %d\n", c->plate, c->floor);
    //push_queue(ex_queues[c->floor], c);
    //pthread_cond_broadcast(&ex_queues_cond);
    free(c);
    return NULL;
}