#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef int (*processFunc)(int, char **);

void copyState(uint64_t **targetStack, uint64_t *sourceStack);
void replaceProcess(processFunc process, int argvc, char *argv[], uint64_t **stack);
void *createInitStack(void *initStack);
void *createWaiterStack(void *waiterStack);
void startSystem();

#endif /* PROCESS_H */