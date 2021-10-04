/************************************************
 * @file    parking.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for parking.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <stdlib.h>     /* for dynamic memory */
#include <string.h>     /* for string operations */
#include <pthread.h>    /* for the mutexes and conditions */
#include <sys/mman.h>   /* for mapping operations */
#include <fcntl.h>      /* for file modes like O_RDWR */
#include <unistd.h>     /* for misc */

#include "parking.h"    /* corresponding header */

void *create_shared_memory(char *name, size_t size) {

    /* remove any previous instance of the shared memory object, if it exists */
    if (shm_unlink(name) == 0) puts("Previous shared memory unlinked");
    
    /* create the shared memory segment for read and write */
    int shm_fd;
    void *shm;

    if ((shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666)) < 0) {
        perror("Could not create shared memory");
        exit(1);
    }

    /* config the size of the shared memory segment */
    ftruncate(shm_fd, size);

    if ((shm = mmap(0, size, PROT_WRITE, MAP_SHARED, shm_fd, 0)) == (char *)-1) {
        perror("mmap failed");
        exit(1);
    }

    return shm;
}

void init_shared_memory(void *shm, int entrances, int exits, int levels) {

    int offset = 0; /* keep track of offset so we don't overwrite memory */

    /* mutex and condition's attributes that allow them to be shared across processes */
    pthread_mutexattr_t mattr;
    pthread_condattr_t cattr;
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

    /* ENTRANCES */
    for (int i = 0; i < entrances; i++) {
        entrance_t *en = malloc(sizeof(entrance_t) * 1);
        /* for debugging...
        strcpy(en->sensor.plate, "123ENT");
        */

        /* apply to mutexes and conditions for this floor */
        pthread_mutex_init(&en->sensor.lock, &mattr);
        pthread_mutex_init(&en->gate.lock, &mattr);
        pthread_mutex_init(&en->sign.lock, &mattr);
        pthread_cond_init(&en->sensor.condition, &cattr);
        pthread_cond_init(&en->gate.condition, &cattr);
        pthread_cond_init(&en->sign.condition, &cattr);

        memcpy((char *)shm + offset, en, sizeof(entrance_t) * 1);
        offset += sizeof(entrance_t);
        /* as we copied items in, we no longer need to keep the original */
        free(en);
    }

    /* for debugging...
    printf("Now initialising exits from byte %d onwards\n", offset);
    */

    /* EXITS */
    for (int i = 0; i < exits; i++) {
        exit_t *ex = malloc(sizeof(exit_t) * 1);

        pthread_mutex_init(&ex->sensor.lock, &mattr);
        pthread_mutex_init(&ex->gate.lock, &mattr);
        pthread_cond_init(&ex->sensor.condition, &cattr);
        pthread_cond_init(&ex->gate.condition, &cattr);

        memcpy((char *)shm + offset, ex, sizeof(exit_t));
        offset += sizeof(exit_t);
        free(ex);
    }

    /* LEVELS */
    for (int i = 0; i < levels; i++) {
        level_t *lvl = malloc(sizeof(level_t) * 1);

        pthread_mutex_init(&lvl->sensor.lock, &mattr);
        pthread_cond_init(&lvl->sensor.condition, &cattr);

        memcpy((char *)shm + offset, lvl, sizeof(level_t));
        offset += sizeof(level_t);
        free(lvl);
    }
}

void destroy_shared_memory(void *shm, size_t size, char *name) {
    if (munmap(shm, size) == -1) perror("munmap failed");
    shm_unlink(name);
}