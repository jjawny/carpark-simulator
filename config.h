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

/* parking spots per level - must be at least 1 */
#define CAPACITY 20

/* duration (seconds) each software will run for */
#define DURATION 20