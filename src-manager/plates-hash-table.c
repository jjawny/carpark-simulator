/************************************************
 * @file    plates-hash-table.c
 * @author  Johnny Madigan
 * @date    September 2021
 * @brief   Source code for 'plates-hash-table' header file
 ***********************************************/
#include <stdio.h>      /* for IO operations */
#include <stdlib.h>     /* for dynamic memory */
#include <string.h>     /* for string operations */
#include <time.h>       /* for clock */
#include <stdbool.h>    /* for bool operations */
#include <ctype.h>      /* for isalpha, isdigit... */

#include "plates-hash-table.h"

htab_t *new_hashtable(size_t h_size) {
    
    htab_t *h = malloc(sizeof(htab_t) * 1);
    h->size = h_size;
    h->buckets = malloc(sizeof(node_t*) * h_size);
    
    for (int i = 0; i < h_size; i++) {
        h->buckets[i] = NULL;
    }
    
    puts("Hash table created/initialised");
    return h;
}

void print_hashtable(htab_t *h) {
    /* for each bucket */
    for (int i = 0; i < h->size; i++) {

        /* if empty bucket? */
        if (h->buckets[i] == NULL) {
            
            printf("%d.\t---\n", i);

        /* print bucket's linked list */
        } else {
            printf("%d.\t", i);
            node_t *slot = h->buckets[i];
            
            while (slot != NULL) {
                printf("%s\t", slot->plate);            
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

void hashtable_add(htab_t *h, char *plate, int assigned_lvl) {

    /* only add if plate is correct length */
    if (strlen(plate) != 6) return;

    /* hash to find where to add node */
    int key = (int)hash(plate, h->size);
    node_t *slot = h->buckets[key];

    /* ensure given plate to uppercase to keep # table case-insensitive */
    int j = 0;
    while (plate[j]) {
        plate[j] = (char)(toupper(plate[j]));
        j++;
    }

    /* if slot is empty (no HEAD), we can insert our plate as the head */
    if (slot == NULL) {
        
        /* set up the new node */
        node_t *new_n = malloc(sizeof(node_t) * 1);
        strcpy(new_n->plate, plate);
        clock_gettime(CLOCK_MONOTONIC_RAW, &new_n->start);
        new_n->assigned_lvl = assigned_lvl;
        new_n->next = NULL;
        
        /* add new node to the hash table */
        h->buckets[key] = new_n;
        return;
    }
    
    /* if there was a COLLISION, traverse to end of linked list and append, 
    also checking if THIS plate already exists and prevent duplicate */
    node_t *prev;
    while (slot != NULL) {
        if (strcmp(slot->plate, plate) == 0) return;
        prev = slot;
        slot = prev->next;
    }

    /* reached here if the plate is NOT a duplicate, and we've
    found the end of the linked list - set up the new node and
    point it to NULL as this is the now the tail node */
    node_t *new_n = malloc(sizeof(node_t) * 1);
    strcpy(new_n->plate, plate);
    clock_gettime(CLOCK_MONOTONIC_RAW, &new_n->start);
    new_n->assigned_lvl = assigned_lvl;
    new_n->next = NULL;

    /* step back by 1 node and point it to our new plate (appending) */
    prev->next = new_n;
}

void hashtable_delete(htab_t *h, char *plate) {

    /* hash to find which bucket node would be at */
    int key = (int)hash(plate, h->size);
    node_t *head = h->buckets[key];
    node_t *current = head;
    node_t *previous = NULL;

    /* ensure given plate to uppercase to keep # table case-insensitive */
    int j = 0;
    while (plate[j]) {
        plate[j] = (char)(toupper(plate[j]));
        j++;
    }

    /* traverse bucket's linked list to find node */
    while (current != NULL) {
        if (strcmp(current->plate, plate) == 0) {
            /* if the plate is the head */
            if (previous == NULL) {
                h->buckets[key] = current->next;
                free(current);
                return;
            
            /* if plate is in the middle/end of the list */
            } else {
                previous->next = current->next;
                free(current);
                return;
            }
        }
        previous = current;
        current = current->next;
    }
    puts("Plate is not in hash table - cannot delete");
}

node_t *hashtable_find(htab_t *h, char *plate) {

    /* hash to find which bucket node would be at */
    int key = (int)hash(plate, h->size);
    node_t *slot = h->buckets[key];

    /* ensure given plate to uppercase to keep # table case-insensitive */
    int j = 0;
    while (plate[j]) {
        plate[j] = (char)(toupper(plate[j]));
        j++;
    }

    /* traverse bucket's linked list and return if found */
    while (slot != NULL) {
        if (strcmp(slot->plate, plate) == 0) {
            return slot;
        }
        slot = slot->next;
    }

    return NULL; /* reached here if not found */
}

bool hashtable_destroy(htab_t *h) {
    
    /* free linked lists */
    for (int i = 0; i < h->size; ++i) {

        node_t *bucket = h->buckets[i]; /* current bucket */

        /* traverse and free until end of list (NULL) */
        while (bucket != NULL) {
            node_t *next = bucket->next;
            free(bucket);
            bucket = next;
        }
    }

    /* free buckets array */
    free(h->buckets);
    h->buckets = NULL;
    h->size = 0;

    return true;
}