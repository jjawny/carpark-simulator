/************************************************
 * @file    display-status.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for displaying the status of all car-park's hardware
 ***********************************************/
#pragma once

/**
 * @brief Formats terminal to elegantly display all
 * car-park's hardware statuses. Refreshes every 50ms
 * to prevent fatigue.
 * 
 * @param args - collection of values
 * @return void* - return NULL upon completion
 */
void *display(void *args);