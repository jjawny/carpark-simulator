/************************************************
 * @file    fire-gate.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for opening all boomgates when there's a fire.
 ***********************************************/
#pragma once

/**
 * @brief In the event of a fire, this function will
 * raise all boomgates - checking every 5 seconds
 * if the gates have been closed to raise them
 * again (if there is still a fire).
 * 
 * @param args - mandatory thread args (unused)
 * @return void* - return NULL upon completion
 */
void *open_gate(void *args);