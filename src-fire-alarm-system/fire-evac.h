/************************************************
 * @file    fire-evac.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for displaying "EVACUATE" on all
 *          signs when there's a fire.
 ***********************************************/
#pragma once

/**
 * @brief In the event of a fire, this function will
 * loop through and update each sign with the letters 
 * in "EVACUATE" every 20ms.
 * 
 * @param args - mandatory thread args (unused)
 * @return void* - return NULL upon completion
 */
void *evac_sign(void *args);