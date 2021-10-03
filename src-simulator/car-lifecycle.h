/************************************************
 * @file    car-lifecycle.h
 * @author  Johnny Madigan
 * @date    October 2021
 * @brief   API for a car's lifecycle. Only to be
 *          called by SIMULATE ENTRANCE THREADS when
 *          letting cars into the carpark. As cars
 *          move individually once inside, each car
 *          needs its own thread, especially for
 *          parking for different times simultaneously.
 ***********************************************/
#pragma once

/**
 * @brief Authorised cars that enter (carpark not full)
 * are given their own thread so they can drive/park
 * simultaneously. Once a car is ready to leave, it
 * will drive to a random exit queuing up to be billed
 * before leaving the Sim - all times are in milliseconds. 
 * 
 * Cars take 10ms to drive to its parking space
 * Cars park for 100..10000ms
 * Cars take 10ms to drive to a random exit
 * 
 * @param car - the car
 * @return void* - NULL upon completion
 */
void *car_lifecycle(void *car);