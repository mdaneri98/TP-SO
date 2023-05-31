#ifndef FILOSOPHER_H
#define FILOSOPHER_H

#include <stdint.h>



typedef enum State {
    THINKING, HUNGRY, EATING
} phylState;


int filosopher(int argsc, char* argsv[]);

#endif