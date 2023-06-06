#ifndef FILOSOPHER_H
#define FILOSOPHER_H

#include <stdint.h>


#define FILO_MUTEX "mutex"

typedef enum State {
    THINKING, HUNGRY, EATING
} phylState;


int filosopher(int argsc, char* argsv[]);

#endif