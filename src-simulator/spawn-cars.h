/************************************************
 * @file    spawn-cars.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for spawning cars with random 
 *          number plates
 ***********************************************/
#pragma once

#include "queue.h" /* for car type */

/**
 * @brief   Spawns a new car every 1..100 milliseconds. 
 *          Cars are given a randomised license plate 
 *          and directed to a random queue.
 * 
 * @param   args - mandatory pthread param, unused in function 
 * @return  void* - mandatory return value (NULL)
 */
void *spawn_cars(void *args);

/**
 * @brief   Helper function for spawning cars. Generates a 
 *          random license plate in the format of 3 digits
 *          and 3 alphabet characters like '111AAA'.
 * 
 * @param   c - car to assign random plate to
 */
void random_plate(car_t *c);