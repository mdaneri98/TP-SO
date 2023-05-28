#ifndef PS_H
#define PS_H

#include <stdint.h>

typedef struct ProcessData {
    uint32_t id;
    void *stack;
    void *baseStack;
    char foreground;
    uint8_t priority;
} ProcessData;

#endif