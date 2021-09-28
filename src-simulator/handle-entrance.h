#pragma once

#include "queue.h"

/* entrance thread args */
typedef struct en_args_t {
    int number;
    void *shared_memory;
    queue_t *queue;
} en_args_t;


void *handle_entrance(void *args);