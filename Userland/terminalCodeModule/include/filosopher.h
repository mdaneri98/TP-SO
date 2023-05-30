#ifndef COLOR_H
#define COLOR_H

#include <stdint.h>

#define mutexName "mutex"

typedef enum State {
    THINKING, HUNGRY, EATING
} phylState;

// Sacar de acá y pasar, cuando esté hecho, a .h de semáforos.
typedef int sem_t;

static void filosopher(int argsc, char* argsv[]);

#endif