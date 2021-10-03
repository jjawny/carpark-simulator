/************************************************
 * @file    display-status.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for printing status of all Sim's hardware
 ***********************************************/
#pragma once

/**
 * @brief Formats terminal to elegantly display
 * all Sim's hardware statuses. Refreshes every 50ms
 * to prevent fatigue.
 * 
 * @param args - mandatory thread args (unused)
 * @return void* - return NULL upon completion
 */
void *display(void *args);