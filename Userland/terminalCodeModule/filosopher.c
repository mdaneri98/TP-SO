
#include <stdio.h>
#include <lib.h>
#include <filosopher.h>
#include <syscalls.h>


#define LEFT (i+n-1) % n
#define RIGHT (i+1) % n

/* Prototypes */

static void redraw(int i, int n, phylState* state, sem_t* mutexSem);
static void take_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t* s);
static void put_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t* s);
static void test(int i, int n, phylState* state, sem_t* s);


/* --------------------- */


char ansiArt[5] = { '.' };

int filosopher(int argsc, char* argsv[]) {
    int i = stringToInt(argsv[1]);
    int n = stringToInt(argsv[2]);

    char semName[] = "mutex";
    sem_t mutexSem = _sysSemOpen(semName, 0);
    sem_t s[n];

    phylState state[n];
    for (int i = 0; i < n; i++) {
        state[i] = HUNGRY;
    }

    /* ¿Donde está el arte? */
    while (TRUE) {
        take_forks(i, n, state, &mutexSem, s);
        redraw(i, n, state, &mutexSem);
        put_forks(i, n, state, &mutexSem, s);
        redraw(i, n, state, &mutexSem);
    }

    return 0;
}

static void redraw(int i, int n, phylState* state, sem_t* mutexSem) {
    _sysSemDown(mutexSem);
    if (state[i] == EATING)
        ansiArt[i] = 'E' ;
    else 
        ansiArt[i] = '.';

    
    for (int i = 0; i < n; i++) {
        printf("%c ", ansiArt[i]);
        printf("\n");
    }
    _sysSemUp(mutexSem);
}

static void take_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t s[]) {
    _sysSemDown(mutexSem);
    state[i] = HUNGRY;
    test(i, n, state, s);
    _sysSemUp(mutexSem);
    _sysSemDown(&s[i]);
}

static void put_forks(int i, int n, phylState* state, sem_t* mutexSem, sem_t s[]) {
    _sysSemDown(mutexSem);
    state[i] = THINKING;
    test(LEFT, n, state, s);
    test(RIGHT, n, state, s);
    _sysSemUp(mutexSem);
}

static void test(int i, int n, phylState* state, sem_t s[]) {
    if (state[i] == HUNGRY && state[LEFT] != EATING && state[RIGHT] != EATING) {
        state[i] = EATING;
        _sysSemUp(&s[i]);
    }
}

