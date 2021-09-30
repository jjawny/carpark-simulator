/************************************************
 * @file    plates-hash-table.h
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   API for creating/manipulating a hash table of 
 *          license plates + other optional data (entry time,
 *          assigned floor).
 * 
 *          Purpose built for checking authorised license plates,
 *          checking which cars are assigned to which levels,
 *          and calculating bill for duration of stay. 
 * 
 *          Querying is case-insensitive so the manager can still 
 *          function properly with human error (such as if plates.txt 
 *          contains lowercase plates). All plates will be converted 
 *          to uppercase before inserting/finding/deleting.
 ***********************************************/
#pragma once

#include <stdio.h>      /* for IO operations */
#include <stdlib.h>     /* for dynamic memory */
#include <string.h>     /* for string operations */
#include <stdbool.h>    /* for bool operations */
#include <ctype.h>      /* for isalpha, isdigit */
#include <sys/time.h>   /* for timeval type */

#define PLATE_SIZE 6

/* Plate type */
typedef struct node_t {
    char plate[PLATE_SIZE];
    struct timeval start_time;
    int assigned_lvl;
    struct node_t *next;
} node_t;

/* # table type */
typedef struct htab_t {
    node_t **buckets;
    size_t size;
} htab_t;

/* as # tables are accessed across multiple threads, rather than 
constantly passing a pointer to an array around (effectively making
it global), we can greatly reduce code complexity by letting the # tables 
be global BUT restrict their access using mutex locks */
extern htab_t *plates_ht;
extern htab_t *bill_ht;
extern pthread_mutex_t plates_ht_lock;
extern pthread_mutex_t bill_ht_lock;

/**
 * @brief Returns a new # table after creating and initialising.
 * 
 * @param h_size - no. of buckets aka size of # table's key col
 * @return htab_t* - pointer to the # table
 */
htab_t *new_hashtable(size_t h_size);

/**
 * @brief Prints a given #table
 * 
 * @param h - # table to print
 */
void print_hashtable(htab_t *h);

/**
 * @brief Hashes a string using the djb2 # function. Slightly
 * modified from CAB403's practical 3 lesson, with further 
 * modifications by myself.
 * @see http://www.cse.yorku.ca/~oz/hash.html for more details.
 * 
 * @param s - string to hash
 * @param h_size - size of the # table to keep keys within bounds
 * @return size_t - hashed key
 */
size_t hash(char *s, size_t h_size);

/**
 * Adds a plate to the hash table. Given the plate, the function
 * will hash it to obtain the 
 * 
 * Dual purpose - set assigned_lvl to 0 if u don't care/need the value
 * 
 * @param hash table to add to
 * @param plate to add
 */

/**
 * @brief Adds a plate to the # table. Given the plate, the function
 * will hash it to obtain the key. If there is a collision, we will
 * traverse the linked-list attached to that bucket and add the plate
 * to the end. While traversing, if the plate is found to be a duplicate
 * we will abandon the process.
 * 
 * @param h - # table to add to
 * @param plate - to add
 * @param assigned_lvl - plate's assigned level (set to 0 if you don't need the value)
 */
void hashtable_add(htab_t *h, char *plate, int assigned_lvl);

/**
 * @brief Delete an entry from the # table. After hashing the plate to obtain
 * the key, we will go to that bucket and delete the plate if it matches
 * the given plate, otherwise, we begin traversing the linked list.
 * 
 * If the plate is the head of a linked list, the plate's next node will
 * become the head. If the plate is in the middle/end, we will join the
 * previous and next plates so there is no dangling pointer. If the plate
 * was never found, no changes are made.
 * 
 * @param h - # table to search
 * @param plate - plate's node to delete
 */
void hashtable_delete(htab_t *h, char *plate);

/**
 * @brief Find an entry from the # table. After hashing the plate to obtain
 * the key, we will go to that bucket and return true if the plate matches
 * the given plate, otherwise, we begin traversing the linked list.
 * 
 * If the plate was never found, we return false.
 * 
 * @param h - # table to search
 * @param plate - plate to find
 * @return true - if found
 * @return false - if not found
 */
bool hashtable_find(htab_t *h, char *plate);

/**
 * @brief Destroy an initialised # table by traversing all linked lists,
 * freeing each node. Then freeing all the buckets.
 * 
 * @param h - # table to destroy
 * @return true - once destroyed
 */
bool hashtable_destroy(htab_t *h);
