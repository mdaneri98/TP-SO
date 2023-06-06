
#include <stdio.h>
#include <lib.h>
#include <filosopher.h>
#include <syscalls.h>
#include <string.h>
#include <shared_variables.h>

#define MAX_LENGTH 255

#define LEFT (index+n-1) % n
#define RIGHT (index+1) % n

/* Prototypes 

static void redraw(int i, int n, phylState* state, sem_t* mutexSem);
static void take_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t* s[]);
static void put_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t* s[]);
static void test(int i, int n, phylState* state, sem_t* s[]);


/* --------------------- */



int filosopher(int argsc, char* argsv[]) {
    return 0;
}
    /*
    int index = stringToInt(argsv[1]);
    int n = stringToInt(argsv[2]);
    
    /* ¿Donde está el arte? 
    while (TRUE) {
        take_forks(index, n, state, mutexSem, s);
        redraw(index, n, state, mutexSem);
        put_forks(index, n, state, mutexSem, s);
        redraw(index, n, state, mutexSem);
    }

    return 0;
}

static void redraw(int index, int n, phylState* state, sem_t* mutexSem) {
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

static void take_forks(int index, int n, phylState* state, sem_t* mutexSem, sem_t* s[]) {
    _sysSemDown(mutexSem);
    state[index] = HUNGRY;
    test(index, n, state, s);
    _sysSemUp(mutexSem);
    _sysSemDown(s[index]);
}

static void put_forks(int index, int n, phylState* state, sem_t* mutexSem, sem_t* s[]) {
    _sysSemDown(mutexSem);
    state[index] = THINKING;
    test(LEFT, n, state, s);
    test(RIGHT, n, state, s);
    _sysSemUp(mutexSem);
}

static void test(int index, int n, phylState* state, sem_t* s[]) {
    if (state[index] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[index] = EATING;
        _sysSemUp(s[index]);
    }
}


*/