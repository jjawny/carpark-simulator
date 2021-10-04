/************************************************
 * @file    manage-gate.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for functions for pure gate threads.
 *          As gates stay open for 20ms while entrances
 *          and exits continue to operate, seperate threads
 *          must be utilised to let this pause happen in
 *          the background.
 ***********************************************/
#pragma once

/**
 * @brief Waits until the corresponding gate is left open,
 * gate must remain open for 20ms before lowering.
 * 
 * @param args - includes thread id + address of entrance
 * @return void* - return NULL upon completion
 */
void *manage_en_gate(void *args);

/**
 * @brief Waits until the corresponding gate is left open,
 * gate must remain open for 20ms before lowering.
 * 
 * @param args - includes thread id + address of entrance
 * @return void* - return NULL upon completion
 */
void *manage_ex_gate(void *args);

/**
 * @brief Sleeps for 'ms' milliseconds
 * 
 * @param ms - milliseconds to sleep
 */
void sleep_for_millis(int ms);
