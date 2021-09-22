
#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <string.h>     /* for string stuff... */
#include <stdbool.h>    /* for bool stuff... */

#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <stdint.h>


#include "parking.h"


/**
 * FORMULAS for locating segments of the PARKING shared memory.
 * Where "i" starts at zero and counts up while "i" < LEVELS.
 * 
 * for entrances:   sizeof(entrance * i)
 * for exits:       sizeof(entrance * LEVELS) + sizeof(exit * i)
 * for floors:      sizeof(entrance * LEVELS) + sizeof(exit * LEVELS) + sizeof(floor * i)
 */


void *create_shared_memory(size_t size) {

    int protection = PROT_READ | PROT_WRITE;
    int visibility = MAP_SHARED | MAP_ANONYMOUS;

    puts("Shared memory created");

    return mmap(NULL, size, protection, visibility, -1, 0);
}

void init_shared_memory(void *memory, int levels) {

    int offset = 0; /* keep track of offset so we don't overwrite memory */

    /* ENTRANCES */
    for (int i = 0; i < levels; i++) {
        entrance_t *en = malloc(sizeof(entrance_t) * 1);
        /* for debugging...
        strcpy(en->sensor.plate, "123ENT");
        */

        /* mutex and condition's attributes that allow them to be shared across processes */
        pthread_mutexattr_t mattr;
        pthread_condattr_t cattr;
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

        /* apply to all mutexes for this entrance */
        pthread_mutex_init(&en->sensor.lock, &mattr);
        pthread_mutex_init(&en->gate.lock, &mattr);
        pthread_mutex_init(&en->sign.lock, &mattr);

        /* apply to all conditions for this entrance */
        pthread_cond_init(&en->sensor.condition, &cattr);
        pthread_cond_init(&en->gate.condition, &cattr);
        pthread_cond_init(&en->sign.condition, &cattr);

        memcpy(memory + offset, en, sizeof(entrance_t) * 1);
        offset += sizeof(entrance_t);
        /* as we copied items in, we no longer need to keep the original */
        free(en);
    }

    /* for debugging...
    printf("Now init exits from %d bytes onwards\n", offset);
    */

    /* EXITS */
    for (int i = 0; i < levels; i++) {
        exit_t *ex = malloc(sizeof(exit_t) * 1);
        /* for debugging...
        strcpy(ex->sensor.plate, "123EXT");
        */

       /* mutex and condition's attributes that allow them to be shared across processes */
        pthread_mutexattr_t mattr;
        pthread_condattr_t cattr;
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

        /* apply to all mutexes for this exit */
        pthread_mutex_init(&ex->sensor.lock, &mattr);
        pthread_mutex_init(&ex->gate.lock, &mattr);

        /* apply to all conditions for this exit */
        pthread_cond_init(&ex->sensor.condition, &cattr);
        pthread_cond_init(&ex->gate.condition, &cattr);

        memcpy(memory + offset, ex, sizeof(exit_t));
        offset += sizeof(exit_t);
        free(ex);
    }

    /* for debugging...
    printf("Now init floors from %d bytes onwards\n", offset);
    */

    /* FLOORS */
    for (int i = 0; i < levels; i++) {
        level_t *lvl = malloc(sizeof(level_t) * 1);
        /* for debugging...
        strcpy(lvl->sensor.plate, "123FLR");
        */

       /* mutex and condition's attributes that allow them to be shared across processes */
        pthread_mutexattr_t mattr;
        pthread_condattr_t cattr;
        pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
        pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

        /* apply to all mutexes for this floor */
        pthread_mutex_init(&lvl->sensor.lock, &mattr);

        /* apply to all conditions for this floor */
        pthread_cond_init(&lvl->sensor.condition, &cattr);

        memcpy(memory + offset, lvl, sizeof(level_t));
        offset += sizeof(level_t);
        free(lvl);
    }

    puts("Shared memory initialised");
}
