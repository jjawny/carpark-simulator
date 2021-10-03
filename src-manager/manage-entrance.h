/************************************************
 * @file    handle-entrance.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for handling an entrance and its
 *          hardware. Used with entrance threads.
 ***********************************************/
#pragma once

/**
 * @brief Manages entrance hardware by validating
 * license plates, checking if carpark is full, 
 * assigning car's to levels, displaying info sign
 * with assigned level, displaying EVACUATION in-case
 * of fire, raises/lowers boom gates, etc.
 * 
 * @param args - collection of args to be deconstructed
 * @return void* - return NULL upon completion
 */
void *manage_entrance(void *args);