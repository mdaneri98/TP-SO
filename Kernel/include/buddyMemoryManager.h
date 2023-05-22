#ifndef BUDDY_MEMORY_MANAGER_H
#define BUDDY_MEMORY_MANAGER_H

#include <stdint.h>
#include <string.h>

#define NULL (void *)0


// If our kernel is 512MB in size, starting from this direction will give us all the available memory in Userspace
#define INITIAL_MEMORY_MANAGER_ADDRESS 0x400000
#define INITIAL_MEMORY_MANAGER_LIMIT 0x1FFFFFFF

void createBuddyMemoryManager(void *const restrict init, uint64_t size);

void *allocMemory(const uint64_t memoryToAllocate);

void*reallocMemory(void *const restrict memoryToRealloc, uint64_t newSize);

void freeMemory(void *const restrict memoryToFree);

uint64_t getPowerOfTwo(uint64_t number);

uint64_t getFreeMemoryAmount();

uint64_t getUsedMemoryAmount();

#endif /* BUDDY_MEMORY_MANAGER_H */