#ifndef FREE_LIST_MEMORY_MANAGER_H
#define FREE_LIST_MEMORY_MANAGER_H

#include <stdint.h>

#define NULL (void *)0

void createDefaultMemoryManager(void *const restrict init, uint64_t size);

void *allocMemory(const uint64_t memoryToAllocate);

void *reAllocMemory(void *const memoryToRealloc, uint64_t newSize);

void *allocPCB();

void freeMemory(void *const memoryToFree);

void freePCB(void *const blockToFree);

uint64_t getFreeMemoryAmount();

uint64_t getUsedMemoryAmount();

#endif /* FREE_LIST_MEMORY_MANAGER_H */