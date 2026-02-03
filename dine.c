#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#ifndef DAWDLEFACTOR
#define DAWDLEFACTOR 1000
#endif

/* Default number of times a philosopher repeats their eat/think cycle */
#define DEFAULT_REPS 1

/* Longest name for philosopher states, in this case its 'Think' */
#define MAX_STATE_NAME 5

/* The number of spacing characters between elements in the printout column */
#define PADDING 1

/* Length of the left side of a column */
#define LEFTPAD PADDING + NUM_PHILOSOPHERS

/* Length of the right side of a column */
#define RIGHTPAD MAX_STATE_NAME + PADDING

/* Length of a column not including its edges */
#define COLUMN_LENGTH LEFTPAD + PADDING + RIGHTPAD

/* Length of the printout table including its edges */
#define TABLE_LENGTH 1 + NUM_PHILOSOPHERS * (COLUMN_LENGTH + 1)

struct Philosopher {
    char name;
    char state[MAX_STATE_NAME + 1];
    
    /* Boolean for the philosopher's left fork, 
     *      zero when not holding and nonzero otherwise */
    int forkl; 
    
    /* Boolean for the philosopher's right fork,
     *      zero when not holding and nonzero otherwise */
    int forkr;
};

/* The table of philisophers ready to eat, initialized as an array */
struct Philosopher *table = NULL;

/* An array of forks, each with their own semaphore */
sem_t *forks = NULL;

void dawdle() {
/*
 * * sleep for a random amount of time between 0 and DAWDLEFACTOR
 * * milliseconds. This routine is somewhat unreliable, since it
 * * doesn’t take into account the possiblity that the nanosleep
 * * could be interrupted for some legitimate reason.
   */
    struct timespec tv;
    int msec = (int)((((double)random()) / RAND_MAX) * DAWDLEFACTOR);
    tv.tv_sec = 0;
    tv.tv_nsec = 1000000 * msec;
    if (-1 == nanosleep(&tv, NULL)) {
        perror("nanosleep");
    }
}

/* Returns the index of the philosopher's left fork 
 * based on their own index
 * No special cases */
int get_left_fork(int id) {
    return id;
}

/* Returns the index of the philosopher's right fork 
 * based on their own index
 * Special case with the last philosopher using the first fork*/
int get_right_fork(int id) {
    int fork_idx = id + 1;
    if (fork_idx == NUM_PHILOSOPHERS) {
           return 0;
    }
    return fork_idx;
}

void print_table(void) {
    int i;
    char pad[PADDING];

    for (i=0;i<PADDING;i++) {
        pad[i] = ' ';
    }

    printf("|");
    
    for (i=0;i<NUM_PHILOSOPHERS;i++) {
        int v;

        int holding_left = table[i].forkl;
        int holding_right = table[i].forkr;
        
        int left_fork = get_left_fork(i);
        int right_fork = get_right_fork(i);

        printf("%s", pad);
        
        for (v=0;v<NUM_PHILOSOPHERS;v++) {
            if (v == left_fork && holding_left) {
                printf("%d", v);
            }
            else if (v == right_fork && holding_right) {
                printf("%d", v);
            }
            else {
                printf("-");
            }
        }
        
        printf("%s%s%s|", pad, table[i].state, pad);
    }
    printf("\n");
}
        

void print_header(void) {
    int i;
    char border[TABLE_LENGTH + 1];
    char labels[TABLE_LENGTH + 1];

    char column_border[COLUMN_LENGTH + 1];
    char left_padding[LEFTPAD + 1];
    char right_padding[RIGHTPAD + 1]; 

    border[0] = '\0';
    labels[0] = '\0';

    column_border[COLUMN_LENGTH] = '\0';
    left_padding[LEFTPAD] = '\0';
    right_padding[RIGHTPAD] = '\0';
    
    for (i=0;i<COLUMN_LENGTH;i++) {
        column_border[i] = '=';

        if (i < LEFTPAD) {
            left_padding[i] = ' ';
        }
        if (i < RIGHTPAD) {
            right_padding[i] = ' ';
        }
    }
 
    strcat(border, "|");
    strcat(labels, "|");

    for (i=0;i<NUM_PHILOSOPHERS;i++) {
        char name[2];
        name[0] = (char) i + 'A';
        name[1] = '\0';

        strcat(border, column_border);
        strcat(border, "|");

        strcat(labels, left_padding);
        strcat(labels, name);
        strcat(labels, right_padding);
        strcat(labels, "|");
    }

    printf("%s\n%s\n%s\n", border, labels, border);
    print_table();
}

int main(int argc, char *argv[]) {
    
    int i;

    /* Number of eat/think cycles a philosopher does */
    int reps;
    
    /* Character id to be passed to each philosopher */
    char id[NUM_PHILOSOPHERS];
    
    /* activations for each philosopher */
    pthread_t pid[NUM_PHILOSOPHERS];

    if (argc < 2) {
        reps = DEFAULT_REPS;
    }
    else {
        reps = atoi(argv[1]);
    }

    if (reps < 1) {
        fprintf(stderr, 
            "Expected a valid amount of repititions, instead got %d\n", reps);
        return 1;
    }
    printf("Reps: %d\n", reps);
    
    table = (struct Philosopher *) calloc(NUM_PHILOSOPHERS, sizeof(struct Philosopher));    
    if (!table) {
        perror("Malloc failed");
        return 1;
    }

    for (i=0; i<NUM_PHILOSOPHERS; i++) {
        id[i] = i;
        table[i].name = 'A' + i;
        strcpy(table[i].state, "     ");

        printf("new phil: %c\n", table[i].name);
    }

    print_header();
    return 0;
}
