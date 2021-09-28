#pragma once

#include "queue.h"

extern _Atomic int end_simulation;

extern pthread_mutex_t en_queues_lock;        /* lock for list of queues as this is global */
extern pthread_mutex_t rand_lock;              /* lock for rand calls as seed is global */


