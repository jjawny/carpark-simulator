/************************************************
 * @file    spawn-cars.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for spawn-cars.h
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <string.h>     /* for string operations */
#include <pthread.h>    /* for thread operations */
#include <stdlib.h>     /* for misc like rand */

#include "spawn-cars.h" /* corresponding header */
#include "sim-common.h" /* for flag & rand lock */
#include "../config.h"  /* for no. of EXITS/ENTRANCES/LEVELS */
#include "queue.h"      /* for queue operations */
#include "sleep.h"      /* for custom millisecond sleep */

void *spawn_cars(void *args) {

    while (!end_simulation) {
        /* lock rand call for random entrance and milliseconds wait */
        pthread_mutex_lock(&rand_lock);
        int pause_spawn = ((rand() % 100) + 1); /* 1..100 */
        int q_to_goto = rand() % ENTRANCES;
        pthread_mutex_unlock(&rand_lock);

        /* wait 1..100 milliseconds before spawning a new car */
        sleep_for_millis(pause_spawn);
        car_t *new_c = malloc(sizeof(car_t) * 1);
        
        /* TOGGLE
         * - fixed plate
         * - true randomness
         * - controlled randomness*/
        //strcpy(new_c->plate, "206WHS");
        //random_plate(new_c);
        random_pool(new_c);

        //printf("%s\n", new_c->plate);

        /* goto random entrance */
        
        pthread_mutex_lock(&en_queues_lock);
        push_queue(en_queues[q_to_goto], new_c);
        pthread_mutex_unlock(&en_queues_lock);
        pthread_cond_broadcast(&en_queues_cond); 
        
        /* after placing each car in a queue, broadcast to all entrance
        threads to check if their queue now has a car waiting, this means
        those threads can wait rather than constantly checking, preventing
        busy waiting */
    }
    return NULL;
}

void random_plate(car_t *c) {
    /* random plate to fill */
    char rand_plate[7];

    /* 3 random numbers */
    for (int i = 0; i < 3; i++) {
        pthread_mutex_lock(&rand_lock);
        char rand_number = "123456789"[rand() % 9];
        pthread_mutex_unlock(&rand_lock);
        rand_plate[i] = rand_number;
    }

    /* 3 random letters */
    for (int i = 3; i < 6; i++) {
        pthread_mutex_lock(&rand_lock);
        char rand_letter = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"[rand() % 26];
        pthread_mutex_unlock(&rand_lock);
        rand_plate[i] = rand_letter;
    }

    /* don't forget the null terminator */
    rand_plate[6] = '\0';

    /* assign to this car */
    strcpy(c->plate, rand_plate);
}

void random_pool(car_t *c) {
    /* POOL #1 authorised plates */
    const char *auth[100] = {"029MZH", "030DWF", "042FMO", "042WCI", "046HKC", "064BYE", "080UPF", "081EGU",
                             "088FSB", "122WIV", "137JEG", "168BUT", "174JJD", "177BLJ", "190PKY", "190VUD",
                             "194FSA", "202FUF", "206WHS", "227IFW", "231SVE", "237RJM", "258ZMG", "260QYO",
                             "261IDD", "264PAY", "265UDF", "270RZO", "288MUO", "290MEN", "321XRD", "333IIC",
                             "333KIC", "333RZU", "340PBT", "349USG", "361ECD", "376DDS", "379ZTQ", "394BDU",
                             "400MOQ", "409DRM", "410OWZ", "418PXR", "431OUV", "437PPK", "438GCC", "447CHZ",
                             "449XLO", "451HLR", "462PQO", "472AGU", "474KHO", "480GML", "481WPQ", "495UQC",
                             "510SLS", "510XTI", "511QKV", "549QHD", "549SPQ", "565LTC", "569ETO", "594QVK",
                             "597ILR", "606WFJ", "615PKL", "615XIX", "621QMB", "621VWC", "624BPP", "633WYV",
                             "646HLJ", "657XVT", "688QHN", "693NMP", "704IQV", "727SWM", "753BMQ", "767FWZ",
                             "775CGD", "799BUO", "803JOW", "810EWP", "819PHA", "830PHA", "856HFF", "867EUY",
                             "883ZYX", "889DTO", "890GKB", "896MZE", "908XNZ", "910NDQ", "917IWU", "927KOB",
                             "931KQD", "933SAD", "934NHK", "936CIO"};

    /* POOL #2 non-authorised plates */
    const char *non_auth[100] = {"666EEE", "001ABC", "002ABC", "003ABC", "004ABC", "004ABC", "005ABC", "005ABC",
                                 "111III", "006ABC", "001ABC", "001ABC", "001ABC", "001ABC", "001ABC", "001ABC",
                                 "018ABC", "007ABC", "039ABC", "040ABC", "041ABC", "042ABC", "043ABC", "044ABC",
                                 "019ABC", "008ABC", "038ABC", "045ABC", "063ABC", "064ABC", "001ABC", "001ABC",
                                 "020ABC", "009ABC", "037ABC", "046ABC", "062ABC", "065ABC", "101ABC", "001ABC",
                                 "021ABC", "010ABC", "036ABC", "047ABC", "061ABC", "066ABC", "101ABC", "001ABC",
                                 "022ABC", "011ABC", "035ABC", "048ABC", "060ABC", "066ABC", "001ABC", "001ABC",
                                 "023ABC", "012ABC", "034ABC", "049ABC", "059ABC", "066ABC", "111ABC", "001ABC",
                                 "024ABC", "013ABC", "033ABC", "050ABC", "058ABC", "066ABC", "111ABC", "001ABC",
                                 "025ABC", "014ABC", "032ABC", "051ABC", "057ABC", "066ABC", "111ABC", "001ABC",
                                 "026ABC", "015ABC", "031ABC", "052ABC", "056ABC", "066ABC", "067ABC", "001ABC",
                                 "027ABC", "016ABC", "030ABC", "053ABC", "055ABC", "066ABC", "069ABC", "001ABC",
                                 "028ABC", "017ABC", "029ABC", "054ABC"};

    /* random index & random pool */
    int pool = 0;
    int index = 0;
    pthread_mutex_lock(&rand_lock);
    index = rand() % 100;
    pool = rand() % 5;  /* change modulo 'n' to change randomness (1/n) */
    pthread_mutex_unlock(&rand_lock);

    /* assign to this car */
    if (pool == 0) {
        /* since there are a finite no. of authorised cars
         * versus millions non-authorised, we will only assign
         * a non-authorised plate 1/n times */
        strcpy(c->plate, non_auth[index]);
    } else {
        strcpy(c->plate, auth[index]);
    }
}