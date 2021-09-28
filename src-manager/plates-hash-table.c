/*******************************************************
 * @file    plates-hash-table.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for 'plates-hash-table' header file
 ******************************************************/
#include <stdio.h>      /* for print, scan... */
#include <stdlib.h>     /* for malloc, free... */
#include <string.h>     /* for string stuff... */
#include <stdbool.h>    /* for bool stuff... */
#include <ctype.h>      /* for isalpha, isdigit... */

#include "plates-hash-table.h"



htab_t *new_hashtable(size_t h_size) {
    
    htab_t *h = malloc(sizeof(htab_t) * 1);
    h->size = h_size;
    h->buckets = malloc(sizeof(plate_t*) * h_size);
    
    for (int i = 0; i < h_size; i++) {
        h->buckets[i] = NULL;
    }
    
    puts("Hash table created/initialised");
    return h;
}

void print_hashtable(htab_t *h) {

    for (int i = 0; i < h->size; i++) {

        /* if empty bucket */
        if (h->buckets[i] == NULL) {
            
            printf("%d.\t---\n", i);

        } else {

            /* for each bucket with a value, walk through its
            entire linked list until we reach the end (NULL) */
            printf("%d.\t", i);
            plate_t *slot = h->buckets[i];
            
            while (slot != NULL) {
                printf("%s\t", slot->value);            
                slot = slot->next;
            }

            printf("\n");
        }
    }
}

size_t hash(char *s, size_t h_size) {

    size_t hash = 5381;
    int c;

    while ((c = *s++) != '\0') {
        // hash = hash * 33 + c
        hash = ((hash << 5) + hash) + c;
    }

    return hash % h_size;
}

void hashtable_add(htab_t *h, char *value) {

    int key = (int)hash(value, h->size);

    plate_t *slot = h->buckets[key];

    /* ensure value to add is uppercase */
    int j = 0;
    while (value[j]) {
        value[j] = (toupper(value[j]));
        j++;
    }

    /* if slot is empty (no HEAD), we can insert our value */
    if (slot == NULL) {
        
        /* setup the plate structure by allocating memory for a plate string 
        then copy the value in. Finally set the next node to NULL */
        plate_t *new_plate = malloc(sizeof(new_plate) * 1);
        strcpy(new_plate->value, value);
        new_plate->next = NULL;
        
        /* add new plate structure to the hash table */
        h->buckets[key] = new_plate;
        return;
    }
    
    /* if there was a COLLISION, traverse to end of linked list and append */
    plate_t *prev;
    
    while (slot != NULL) {
        
        /* check for a duplicate value at any point and abandon if so */
        if (strcmp(slot->value, value) == 0) {
            return;
        }
        
        prev = slot;
        slot = prev->next;
    }

    /* reached here if the value is unique and we've found the end of the list */
     
    /* setup the plate structure by allocating memory for a plate string 
    then copy the value in. Finally set the next node to NULL */
    plate_t *new_plate = malloc(sizeof(new_plate) * 1);
    strcpy(new_plate->value, value);
    new_plate->next = NULL;

    /* step back by 1 node and point it to our new value (appending) */
    prev->next = new_plate;
}

void hashtable_delete(htab_t *h, char *value) {

    int key = (int)hash(value, h->size);

    plate_t *head = h->buckets[key];
    plate_t *current = head;
    plate_t *previous = NULL;

    /* ensure value to delete is uppercase */
    int j = 0;
    while (value[j]) {
        value[j] = (toupper(value[j]));
        j++;
    }

    while (current != NULL) {
        if (strcmp(current->value, value) == 0) {
            if (previous == NULL) {
                /* if the value is the head */
                h->buckets[key] = current->next;
                free(current);
                return;
            }
            else {
                /* if value is in the middle/end of the list */
                previous->next = current->next;
                free(current);
                return;
            }
        }
        previous = current;
        current = current->next;
    }
    puts("Value is not in hash table - cannot delete");
}

bool hashtable_find(htab_t *h, char *value) {

    int key = (int)hash(value, h->size);

    plate_t *slot = h->buckets[key];

    /* ensure value to find is uppercase */
    int j = 0;
    while (value[j]) {
        value[j] = (toupper(value[j]));
        j++;
    }

    while (slot != NULL) {

        if (strcmp(slot->value, value) == 0) {
            return true; /* reached here if found */
        }

        slot = slot->next;
    }

    return false; /* reached here if not found */
}

bool hashtable_destroy(htab_t *h) {
    
    /* free linked lists */
    for (int i = 0; i < h->size; ++i) {

        plate_t *bucket = h->buckets[i]; /* current bucket */

        /* traverse and free until end of list (NULL) */
        while (bucket != NULL) {
            plate_t *next = bucket->next;
            free(bucket);
            bucket = next;
        }
    }

    /* free buckets array */
    free(h->buckets);
    h->buckets = NULL;
    h->size = 0;

    puts("Hash table destroyed!");
    return true;
}