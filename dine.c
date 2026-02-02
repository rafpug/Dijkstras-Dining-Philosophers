#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#ifndef NUM_PHILOSOPHERS
#define NUM_PHILOSOPHERS 5
#endif

#ifndef DAWDLEFACTOR
#define DAWDLEFACTOR 1000
#endif

/* Default number of times a philosopher repeats their eat/think cycle */
#define DEFAULT_REPS 1

/* Longest name for philosopher states, in this case its 'Think\0' */
#define MAX_STATE_NAME 6

struct Philosopher {
    char name;
    char state[MAX_STATE_NAME];
    
    /* Boolean for the philosopher's left fork, 
     *      zero when not holding and nonzero otherwise */
    int forkl; 
    
    /* Boolean for the philosopher's right fork,
     *      zero when not holding and nonzero otherwise */
    int forkr;
};

/* The table of philisophers ready to eat, initialized as an array */
struct Philosopher *table = NULL;

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
    return 0;
}
