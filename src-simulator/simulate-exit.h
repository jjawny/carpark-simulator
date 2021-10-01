/************************************************
 * @file    simulate-exit.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for simulating an exit and its
 *          hardware. Used with exit threads.
 ***********************************************/
#pragma once

/**
 * @brief Simulates the cars at an exit and the hardware
 * interactions. Communicating with manager to bill car
 * before opening boomgates.
 * 
 * @param args - collection of items to be deconstructed and used
 * @return void* - NULL upon completion
 */
void *simulate_exit(void *args);