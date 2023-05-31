#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef int (*processFunc)(int, char **);

void setNewStack(uint64_t *targetStack);
int setProcess(processFunc process, int argc, char **argv, void *stack);
void *createInitStack(void *initStack);
void *createIdleStack(void *waiterStack);
void startSystem();

#endif /* PROCESS_H */