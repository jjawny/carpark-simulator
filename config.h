/************************************************
 * @file    config.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Configuration file so clients can easily 
 *          recompile the software pieces to accommodate
 *          different carparks
 ***********************************************/
#pragma once /* please do not touch this line */

/* 1..5 inclusive for each */
#define ENTRANCES 5
#define EXITS 5
#define LEVELS 5

/* Parking spots per level - must be at least 1 */
#define CAPACITY 20

/* Chance for each car spawned to have a random authorised plate */
/* 0..1 inclusive, for example 0.5 = 50% */
#define CHANCE 0.5

/* Duration (seconds) each software will run for */
#define DURATION 9