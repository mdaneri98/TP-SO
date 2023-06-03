/*************************************************************************************************************************
 * Process.h:
 * This header has the purpouse to define functions related to the stack manipulation of the process created with 
 * fork and exec, also for the creation of the first running process and the idle.
*************************************************************************************************************************/

#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef int (*processFunc)(int, char **);

void _setNewStack(uint64_t *targetStack);
int _setProcess(processFunc process, int argc, char **argv, void *stack);
void *_createInitStack(void *initStack);
void *_createIdleStack(void *waiterStack);

#endif /* PROCESS_H */