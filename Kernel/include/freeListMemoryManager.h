#ifndef FREE_LIST_MEMORY_MANAGER_H
#define FREE_LIST_MEMORY_MANAGER_H

#include <stdint.h>

#define NULL (void *)0


// If our kernel is 512MB in size, starting from this direction will give us all the available memory in Userspace
#define INITIAL_MEMORY_MANAGER_ADDRESS 0x400000
#define INITIAL_MEMORY_MANAGER_LIMIT 0x1FFFFFFF

void createDefaultMemoryManager(void *const restrict init, uint64_t size);

void *allocMemory(const uint64_t memoryToAllocate);

void freeMemory(void *const restrict memoryToFree);

uint64_t getFreeMemoryAmount();

uint64_t getUsedMemoryAmount();

#endif /* FREE_LIST_MEMORY_MANAGER_H */