
#include <stdio.h>
#include <lib.h>
#include <filosopher.h>
#include <syscalls.h>
#include <string.h>
#include <shared_variables.h>

#define MAX_LENGTH 255

#define LEFT (i+n-1) % n
#define RIGHT (i+1) % n

/* Prototypes */

static void redraw(int i, int n, phylState* state, sem_t* mutexSem);
static void take_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t* s);
static void put_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t* s);
static void test(int i, int n, phylState* state, sem_t* s);


/* --------------------- */



int filosopher(int argsc, char* argsv[]) {
    int i = stringToInt(argsv[1]);
    int n = stringToInt(argsv[2]);
    
    /* ¿Donde está el arte? */
    while (TRUE) {
        take_forks(i, n, state, mutexSem, s);
        redraw(i, n, state, mutexSem);
        put_forks(i, n, state, mutexSem, s);
        redraw(i, n, state, mutexSem);
    }

    return 0;
}

static void redraw(int i, int n, phylState* state, sem_t* mutexSem) {
    _sysSemWait(mutexSem);
    if (state[i] == EATING)
        ansiArt[i] = 'E' ;
    else 
        ansiArt[i] = '.';

    
    for (int i = 0; i < n; i++) {
        printf("%c ", ansiArt[i]);
        printf("\n");
    }
    _sysSemPost(mutexSem);
}

static void take_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t s[]) {
    _sysSemWait(mutexSem);
    state[i] = HUNGRY;
    test(i, n, state, s);
    _sysSemPost(mutexSem);
    _sysSemWait(s[i]);
}

static void put_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t s[]) {
    _sysSemWait(mutexSem);
    state[i] = THINKING;
    test(LEFT, n, state, s);
    test(RIGHT, n, state, s);
    _sysSemPost(mutexSem);
}

static void test(int i, int n, phylState* state, sem_t s[]) {
    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[i] = EATING;
        _sysSemPost(s[i]);
    }
}

