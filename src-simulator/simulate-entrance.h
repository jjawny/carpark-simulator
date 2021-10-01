/************************************************
 * @file    simulate-entrance.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for simulating an entrance and its
 *          hardware. Used with entrance threads.
 ***********************************************/
#pragma once

/**
 * @brief Simulates the cars at an entrance and the hardware
 * interactions. Requires feedback from the Manager software
 * for verification, automation, and decision-making.
 * 
 * @param args - collection of items to be deconstructed and used
 * @return void* - NULL upon completion
 */
void *simulate_entrance(void *args);