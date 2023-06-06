#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <stdint.h>


void createMemoryManager(void *const restrict init, uint64_t size);

void *allocMemory(const uint64_t memoryToAllocate);

void *reAllocMemory(void *memoryToRealloc, uint64_t newSize);

void *allocPCB();

void *allocSemaphore();

void *allocBuffer();

void *allocTimer();

void freeMemory(void const *memoryToFree);

void freePCB(void const *blockToFree);

void freeSemaphore(void const *semToFree);

void freeBuffer(void const *bufferToFree);

void freeTimer(void const *timerToFree);

void copyBlocks(void const *target, void const *source);

uint64_t getFreeMemoryAmount();

uint64_t getUsedMemoryAmount();

#endif /* MEMORY_MANAGER_H */