/************************************************
 * @file    handle-entrance.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for handling an entrance and its
 *          hardware. Used with entrance threads.
 ***********************************************/
#pragma once

#include "queue.h" /* for queue type in args */

/* entrance thread args */
typedef struct en_args_t {
    int number;
    void *shared_memory;
    queue_t *queue;
} en_args_t;

/**
 * @brief Handles the cars at an entrance and the hardware
 * interactions. Requires feedback from the Manager software
 * for verification, automation, and decision-making.
 * 
 * @param args - collection of items to be deconstructed and used
 * @return void* - NULL upon completion
 */
void *handle_entrance(void *args);