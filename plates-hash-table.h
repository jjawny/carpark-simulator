/*******************************************************
 * @file    plates-hash-table.h
 * @brief   API for creating/manipulating a hash table of license plates
 * @author  Johnny Madigan
 * @date    September 2021
 ******************************************************/
#pragma once

#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <string.h>     /* for string stuff... */
#include <stdbool.h>    /* for bool stuff... */
#include <ctype.h>      /* for isalpha, isdigit... */

#define PLATE_SIZE 6

/* Plate type */
typedef struct plate_t {
    char value[PLATE_SIZE];
    struct plate_t *next;
} plate_t;

/* Hash Table type */
typedef struct htab_t {
    plate_t **buckets;
    int size;
} htab_t;

/**
 * Returns a new hash table after creating and initialising it.
 * 
 * @param amount of buckets (size of the table's key column)
 * @return pointer to the hash table
 */
htab_t *new_hashtable(int h_size);

/**
 * Prints a given hash table.
 * 
 * @param hash table to print
 */
void print_hashtable(htab_t *h);

/**
 * Hashes a string using the djb2 hash function.
 * Slightly modified from CAB403's prac 3 lesson, further modified by myself.
 * 
 * @see http://www.cse.yorku.ca/~oz/hash.html for more details
 * @param string to hash
 * @param size of the table to keep hashed key within bounds using modulo
 * @return hashed key
 */
size_t hash(char *s, int h_size);

/**
 * Adds a value to the hash table. Given the value, the function
 * will hash it to obtain the key. If there is a collision, we will
 * traverse the linked-list attached to that bucket and add the value
 * to the end. While traversing, if the value is found to be a duplicate
 * we will abandon the process.
 * 
 * @param hash table to add to
 * @param value to add
 */
void hashtable_add(htab_t *h, char *value);

/**
 * Delete an entry from the hash table. After hashing the value to obtain
 * the key, we will go to that bucket and delete the value if it matches
 * the given value, otherwise, we begin traversing the linked list.
 * 
 * If the value is the head of a linked list, the value's next node will
 * become the head. If the value is in the middle/end, we will join the
 * previous and next values so there is no dangling pointer. If the value
 * was never found, no changes are made.
 * 
 * @param hash table to search
 * @param value to delete
 */
void hashtable_delete(htab_t *h, char *value);

/**
 * Find an entry from the hash table. After hashing the value to obtain
 * the key, we will go to that bucket and return true if the value matches
 * the given value, otherwise, we begin traversing the linked list.
 * 
 * If the value was never found, we return false.
 * 
 * @param hash table to search
 * @param value to find
 * @return true if found, false if not found
 */
/* find an entry in a hash table*/
bool hashtable_find(htab_t *h, char *value);
