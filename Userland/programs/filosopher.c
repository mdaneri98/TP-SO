
#include <stdio.h>
#include <lib.h>
#include <filosopher.h>
#include <syscalls.h>
#include <string.h>
#include <constants.h>
#include <shared_variables.h>

#define MAX_LENGTH 255

#define LEFT (index+n-1) % n
#define RIGHT (index+1) % n


/* Prototypes */

static void redraw(int i, int n, phylState* state, char* mutex);
static void take_forks(int i, int n, phylState* state, char* mutex, char* s[]);
static void put_forks(int i, int n, phylState* state, char* mutex, char* s[]);
static void test(int i, int n, phylState* state, char* s[]);


/* --------------------- */

int filosopher(int argsc, char* argsv[]) {
    
    int index = stringToInt(argsv[1]);
    int n = stringToInt(argsv[2]);
    
    char semName[BUFFER_MAX_LENGTH];
    stringCopy(semName, BUFFER_MAX_LENGTH, s[index]);

    /* ¿Where's the art? */
    while (TRUE) {
        take_forks(index, n, state, FILO_MUTEX, s);
        redraw(index, n, state, FILO_MUTEX);
        put_forks(index, n, state, FILO_MUTEX, s);
        redraw(index, n, state, FILO_MUTEX);
    }

    return 0;
}

static void redraw(int index, int n, phylState* state, char* mutexSem) {
    _sysSemDown(mutexSem);
    if (state[index] == EATING)
        ansiArt[index] = 'E' ;
    else 
        ansiArt[index] = '.';

    
    for (int j = 0; j < n; j++) {
        putChar(ansiArt[j]);
    }
    printf("\n");
    _sysSemUp(mutexSem);
}

static void take_forks(int index, int n, phylState* state, char* mutexSem, char* s[]) {
    _sysSemDown(mutexSem);
    state[index] = HUNGRY;
    test(index, n, state, s);
    _sysSemUp(mutexSem);
    _sysSemDown(s[index]);
}

static void put_forks(int index, int n, phylState* state, char* mutexSem, char* s[]) {
    _sysSemDown(mutexSem);
    state[index] = THINKING;
    test(LEFT, n, state, s);
    test(RIGHT, n, state, s);
    _sysSemUp(mutexSem);
}

static void test(int index, int n, phylState* state, char* s[]) {
    if (state[index] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[index] = EATING;
        _sysSemUp(s[index]);
    }
}


