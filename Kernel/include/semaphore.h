#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include <string.h>

#define NULL (void *)0

void semLock(uint64_t* sem, uint64_t currentValue);
void semUnLock(uint64_t * currentValue);

#endif /* SEMAPHORE_H */