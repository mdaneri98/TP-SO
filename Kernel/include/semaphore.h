#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include <string.h>

#define NULL (void *)0

void semLock(uint64_t* semValue, uint64_t * mutex);
void semUnLock(uint64_t * semValue, uint64_t * mutex);

#endif /* SEMAPHORE_H */