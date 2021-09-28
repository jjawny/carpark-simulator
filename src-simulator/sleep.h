/*******************************************************
 * @file    sleep.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for temporarily sleeping threads/actions
 ******************************************************/
#pragma once

/**
 * Pause for seconds + nanoseconds.
 * 
 * @param seconds
 * @param nanoseconds
 */
void sleep_for(int s, int ns);