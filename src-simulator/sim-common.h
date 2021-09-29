/************************************************
 * @file    sim-common.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Common items for Simulator's threads.
 ***********************************************/
#pragma once

extern _Atomic int end_simulation;  /* global flag for notifying threads to stop */
extern pthread_mutex_t rand_lock;   /* lock for rand calls as seed is global */