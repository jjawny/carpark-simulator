/************************************************
 * @file    config.h
 * @author  Johnny Madigan
 * @date    October 2021
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
#define DURATION 60

/* Temperature window - min..max degrees inclusive */
/* Both values must be at least 1 and max should be >= min */
/* If 90% of the recent 30 smoothed temperatures are 58+ degrees, this will trigger the fire alarm */
/* If the difference is 8+ degrees between the oldest and latest smoothed temperatures, this will also trigger the fire alarm */
/* NORMAL NO FIRE:  26..33 */
/* TRIGGER RISE:    52..59 */
/* TRIGGER SPIKE    26..46 */
#define MIN_TEMP 26
#define MAX_TEMP 33


/* Slows down all timings by multiplying milliseconds by this no. */
/* Does not affect DURATION */
/* Must be at least 1 */
#define SLOW_MOTION 1