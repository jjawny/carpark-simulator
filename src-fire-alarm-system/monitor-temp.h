/************************************************
 * @file    monitor-temp.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for monitoring a level's temperature
 *          sensor, using 2 algorithms to detect if
 *          there's a possible fire, if so? alert
 *          all other threads via a shared global
 *          variable "alarm_active".
 ***********************************************/
#pragma once

#define MEDIAN_WINDOW 5     /* median of 5 most recent temps = next smoothed temp */
#define SMOOTHED_WINDOW 30  /* only keep the most recent 30 smoothed temps */

/* linkedlist node */
typedef struct node_t {
    int temp;
    struct node_t *next;
} node_t;

/**
 * @brief Monitor the temperature sensor of a level. After collecting 5
 * raw temperatures, find the median which will be the smoothed temp. Repeat
 * until we have collected 30 smoothed temperatures.
 * 
 * Uses 2 algorithms for detecting fires using the most recent 30 smoothed temps.
 * 
 * 1.   If 90% of them are 58+ degrees, trigger the alarm as 
 *      there is a HIGH RISE in temperature indicating a fire.
 * 
 * 2.   If the most recent smoothed temp is 8+ degrees higher than
 *      the oldest smoothed temp, trigger the alarm as there is
 *      a SPIKE in temperature indicating a fire.
 * 
 * @param args - thread id to help locate corresponding level
 * @return void* - return NULL upon completion
 */
void *monitor_temp(void *args);

/**
 * @brief Given an indicator of the alarm's state (int active),
 * if active, the function will toggle all shared memory level's alarms
 * to '1', otherwise toggle all to '0'.
 * 
 * @param active - indicate if alarm is active, 0 = no, 1 = yes
 */
void toggle_all_alarms(int active);

/**
 * @brief Traverse/count nodes in a linked-list and delete (free)
 * all nodes after a certain number of nodes.
 * 
 * @param head - head of the linked-list
 * @param after - delete all nodes after nth node
 */
void delete_after(node_t *head, int after);

/**
 * @brief Traverse a linked-list to free all of its nodes
 * 
 * @param head - head of the linked-list
 */
void free_nodes(node_t *head);

/**
 * @brief Bubble sorting algorithm, iterating from
 * left to right swapping values to ensure ascending
 * order. Once in ascending order, the median will
 * always be in the middle index of the array.
 * 
 * @param arr - first item in integer array
 * @param size - number of items in array
 * @return int - the median of the array (middle val)
 */
int bubble_sort(int *arr, int size);