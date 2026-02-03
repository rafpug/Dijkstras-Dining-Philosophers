/* SOURCE FILE: dine.c
 * Made by: Rafael Salcedo
 *
 * This project was about implementing a solution to 
 * Dijkstras' dining philosophers problem.
 *
 * The problem is that we have a circular table of philosophers
 * who have a bowl of pasta in front of them. There is
 * one fork on the table for every philosopher. Philosophers need
 * two forks to eat their food and refuse to release forks they've
 * grabbed until they finish eating. Then they release the forks
 * and think until they are hungry again
 *
 * This solution uses POSIX semaphores to control individual forks
 * and uses threads to simulate the philosophers.
 *
 * You can control the number of philosophers and the number of times
 * they repeat the cycle of eating and thinking
 *
 */
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

#define BASE_PHIL_NAME 'A'

#define PSHARED 0

#define INIT_VALUE 1

/* boolean for whether a fork is being held */
#define HELD 1

struct Philosopher {
    char name;
    char state[MAX_STATE_NAME + 1];
    
    /* Boolean for the philosopher's left fork, 
     *      zero when not holding and nonzero otherwise */
    int forkl; 
    
    /* Boolean for the philosopher's right fork,
     *      zero when not holding and nonzero otherwise */
    int forkr;

    int reps;
};

/* The table of philisophers ready to eat, initialized as an array */
struct Philosopher *table = NULL;

/* An array of forks, each with their own semaphore */
sem_t *forks = NULL;

/* Semaphore that blocks when a philosphor is changing states
 * This allows for printing individual state changes */
sem_t printing;

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
    char pad[PADDING+1];
    pad[PADDING] = '\0';

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
        name[0] = (char) i + BASE_PHIL_NAME;
        name[1] = '\0';

        strcat(border, column_border);
        strcat(border, "|");

        strcat(labels, left_padding);
        strcat(labels, name);
        strcat(labels, right_padding);
        strcat(labels, "|");
    }

    printf("%s\n%s\n%s\n", border, labels, border);
}

void *dine(void *id) {
    int whoami = *(int*)id;
    int left_fork = get_left_fork(whoami);
    int right_fork = get_right_fork(whoami);
    
    while (table[whoami].reps > 0) {
        sem_t *first_fork;
        sem_t *second_fork;

        int *fork1;
        int *fork2;
        
        /* Decide which fork to take first to avoid deadlock */
        if (whoami % 2 == 0) {
            /* Even philosophers take the right fork first */
            first_fork = &forks[right_fork];
            second_fork = &forks[left_fork];

            fork1 = &table[whoami].forkr;
            fork2 = &table[whoami].forkl;
        }
        else {
            /* Odd philosophers take the left fork first */
            first_fork = &forks[left_fork];
            second_fork = &forks[right_fork];

            fork1 = &table[whoami].forkl;
            fork2 = &table[whoami].forkr;
        }

        /* Blocks until first fork */
        sem_wait(first_fork);
            
        sem_wait(&printing);
        *fork1 = HELD;
        print_table();
        sem_post(&printing);
        
        /* Blocks until second fork */
        sem_wait(second_fork);

        sem_wait(&printing);
        *fork2 = HELD;
        print_table();
        sem_post(&printing);

        /* The philosopher can now begin eating with their two forks */

        sem_wait(&printing);
        strcpy(table[whoami].state, "Eat  ");
        print_table();
        sem_post(&printing);
        
        /* Philosophers take a random amount of time eating */
        dawdle();
        
        /* Philospher finished eating */
        sem_wait(&printing);
        strcpy(table[whoami].state, "     ");
        print_table();
        sem_post(&printing);

        /* Release our second fork */
        sem_post(second_fork);

        sem_wait(&printing);
        *fork2 = !HELD;
        print_table();
        sem_post(&printing);
        
        /* Release our first fork */
        sem_post(first_fork);
        
        sem_wait(&printing);
        *fork1 = !HELD;
        print_table();
        sem_post(&printing);

        /* Philosphers can now freely think for a random amount of time */
        sem_wait(&printing);
        strcpy(table[whoami].state, "Think");
        print_table();
        sem_post(&printing);
        
        dawdle();

        /* Thinking philosophers will become hungry after thinking */
        sem_wait(&printing);
        strcpy(table[whoami].state, "     ");
        print_table();
        sem_post(&printing);

        table[whoami].reps -= 1;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    
    int i;

    /* Number of eat/think cycles a philosopher does */
    int reps;
    
    /* Character id to be passed to each philosopher */
    int ids[NUM_PHILOSOPHERS];
    
    /* activations for each philosopher */
    pthread_t philosophers[NUM_PHILOSOPHERS];
    
    /* Sets a unique seed */
    srandom(time(NULL));
    
    if (argc < 2) {
        reps = DEFAULT_REPS;
    }
    else {
        reps = atoi(argv[1]);
    }

    if (reps < 1) {
        fprintf(stderr, 
            "Expected a valid amount of repititions, instead got %d\n", reps);
        return EXIT_FAILURE;
    }
    
    table = (struct Philosopher *) calloc(NUM_PHILOSOPHERS, sizeof(struct Philosopher));    
    if (!table) {
        perror("Calloc failed");
        exit(EXIT_FAILURE);
    }

    forks = (sem_t *) malloc(NUM_PHILOSOPHERS * sizeof(sem_t));
    if (!forks) {
        perror("Malloc failed");
        free(table);
        exit(EXIT_FAILURE);
    }

    if (sem_init(&printing, PSHARED, INIT_VALUE)) {
        perror("Failed to init semaphore");
        free(forks);    
        free(table);
    }


    for (i=0; i<NUM_PHILOSOPHERS; i++) {
        int err;

        ids[i] = i;

        table[i].name = BASE_PHIL_NAME + i;
        strcpy(table[i].state, "     ");
        table[i].reps = reps;

        err = sem_init(&forks[i], PSHARED, INIT_VALUE);
        if (err) {
            perror("Failed to init semaphore");
            free(forks);
            free(table);
            exit(EXIT_FAILURE);
        }
    }

    print_header();
    print_table();

    for (i=0; i<NUM_PHILOSOPHERS; i++) {
        int res;
        res = pthread_create(
                        &philosophers[i],
                        NULL,
                        dine,
                        (void *) &ids[i]);
    
        if (res == -1) {
            fprintf(stderr, "Child %i: %s\n", i, strerror(res));
            free(forks);
            free(table);
            exit(EXIT_FAILURE);
        }
    }

    for(i=0; i<NUM_PHILOSOPHERS; i++) {
        pthread_join(philosophers[i], NULL);
        sem_destroy(&forks[i]);
    }
    sem_destroy(&printing);

    print_header();

    free(forks);
    free(table);
    return EXIT_SUCCESS;
}
