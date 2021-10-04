/************************************************
 * @file    manage-exit.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for handling an exit and its
 *          hardware. Used with exit threads.
 ***********************************************/
#pragma once

/**
 * @brief Manages entrance hardware by calculating
 * time to bill car and append a file. Then raise gate, 
 * where Sim will allow the car to leave, before lowering
 * gate.
 * 
 * @param args - collection of args to be deconstructed
 * @return void* - return NULL upon completion
 */
void *manage_exit(void *args);

/**
 * @brief Opens (or creates file if it does not exist) for appending.
 * Each line consists of a car's license plate followed by how much
 * they are billed for their stay.
 * 
 * @param name - name of file to open/create 
 * @param plate - plate to write
 * @param bill - $amount to write
 */
void write_file(char *name, char *plate, double bill);