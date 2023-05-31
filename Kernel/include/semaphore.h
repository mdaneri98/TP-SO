#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <stdint.h>
#include <string.h>


void semLock(uint64_t* currentValue, uint64_t* mutex);
void semUnLock(uint64_t* currentValue);

#endif /* SEMAPHORE_H */