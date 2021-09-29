

#include "queue.h"

void *car_lifecycle(void *car) {
    car_t *c = (car_t *)car;

    puts("I'M ALIVE");

    printf("i %s have been told to goto floor: %d\n", c->plate, c->floor);
    free(c);
    return NULL;
}