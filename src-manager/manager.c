// gcc -o ../MANAGER manager.c plates-hash-table.c -Wall

#include <stdio.h>
#include <stdlib.h>

#include "plates-hash-table.h"
#include "../config.h"

#define TABLE_SIZE 100          /* buckets for license plates */


int main(int argc, char **argv) {
    //----------HASH TABLE BELOW------------
    htab_t *plates_ht = new_hashtable(TABLE_SIZE);
    
    FILE *fp = fopen("plates.txt", "r");
    if (fp == NULL) {
        perror("Unable to open athorised number plates file");
        exit(EXIT_FAILURE);
    }

    char line[1000]; /* ensure whole line is read */

    /* read and add to hash table line by line */
    while (fgets(line, sizeof(line), fp) != NULL) {


        /* scan line for first occurance of newline char 
        and replace with null terminator */
        line[strcspn(line, "\n")] = 0;

        bool is_valid = true;

        /* check line is legit size*/
        if (strlen(line) != PLATE_SIZE) {
            printf("%s is an invalid plate\n", line);
            is_valid = false;
        }

        /* check if line is correct format */
        for (int i = 0; i < (PLATE_SIZE / 2); i++) {
            if (isdigit(i) != 0) {
                printf("%s is an invalid plate\n", line);
                is_valid = false;

            }
        }

        for (int i = (PLATE_SIZE / 2); i < PLATE_SIZE; i ++) {
            if (isalpha(i) != 0) {
                printf("%s is an invalid plate\n", line);
                is_valid = false;
            }
        }

        if (is_valid) {
            
            hashtable_add(plates_ht, line);
        }
    }

    fclose(fp);
    //print_hashtable(plates_ht);
    //----------HASH TABLE ABOVE------------


    /* destroy license plates' hash table */
    hashtable_destroy(plates_ht);
    return EXIT_SUCCESS;
}


/*

The roles of the manager:
● Monitor the status of the LPR sensors and keep 
track of where each car is in the car park

● Tell the boom gates when to open and when to close 
(the boom gates are a simple piece of hardware that can 
only be told to open or close, so the job of automatically
closing the boom gates after they have been open for a little while is up to the
manager)

● Control what is displayed on the information signs at each entrance

● As the manager knows where each car is, it is the manager’s job to ensure that there
is room in the car park before allowing new vehicles in (number of cars < number of
levels * the number of cars per level). The manager also needs to keep track of how
full the individual levels are and direct new cars to a level that is not fully occupied

● Keep track of how long each car has been in the parking lot and produce a bill once
the car leaves.

● Display the current status of the parking lot on a frequently-updating screen, showing
how full each level is, the current status of the boom gates, signs, temperature
sensors and alarms, as well as how much revenue the car park has brought in so far

*/