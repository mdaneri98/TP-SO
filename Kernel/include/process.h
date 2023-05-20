#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>

typedef int (*processFunc)(int, char **);

void copyState(uint64_t **targetStack, uint64_t *sourceStack);
void replaceProcess(processFunc process, int argvc, char *argv[], uint64_t **stack);

/* */
void switchProcess(uint32_t pid);

#endif /* PROCESS_H */