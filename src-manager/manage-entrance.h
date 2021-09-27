#pragma once

#include "manage-entrance.h"
#include "parking-types.h"
#include "plates-hash-table.h"
#include "common.h"
#include "../config.h"

/* entrance thread args */
typedef struct en_args_t {
    int number;
    void *shared_memory;
} en_args_t;

void *manage_entrance(void *args);