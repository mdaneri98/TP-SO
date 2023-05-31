#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include <string.h>


void semLock(uint64_t* semValue, uint64_t * mutex);
void semUnlock(uint64_t * semValue, uint64_t * mutex);

#endif /* SEMAPHORE_H */