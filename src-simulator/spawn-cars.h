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
 *          This is true randomness.
 * 
 * @param   c - car to assign random plate to
 */
void random_plate(car_t *c);

/**
 * @brief   Helper function for spawning cars. Creates random
 *          value 'n' 0..100. If n is less than the Chance value
 *          the car is randomly assigned one of the authorised plates.
 *          The greater the Chance, the more likely 'n' will be less
 *          than Chance.
 *
 * @param c - car to assign random plate to
 * @param chance - how likely car receives an authorised license plate
 */
void random_chance(car_t *c, float chance);