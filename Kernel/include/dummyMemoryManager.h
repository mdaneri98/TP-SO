#ifndef DUMMY_MEMORY_MANAGER_H
#define DUMMY_MEMORY_MANAGER

#include <stdint.h>

#define NULL (void *)0


// If our kernel is 512MB in size, starting from this direction will give us all the available memory in Userspace
#define INITIAL_MEMORY_MANAGER_ADDRESS 0x400000
#define INITIAL_MEMORY_MANAGER_LIMIT 0x1FFFFFFF

typedef struct MemoryManagerCDT *MemoryManagerADT;

MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory, uint64_t size);

void *allocMemory(MemoryManagerADT const restrict memoryManager, const uint64_t memoryToAllocate);

void freeMemory(MemoryManagerADT const restrict memoryManager, void *const restrict memoryToFree, const uint64_t memorySize);

uint64_t getFreeMemoryAmount(MemoryManagerADT const restrict memoryManager);

uint64_t getUsedMemoryAmount(MemoryManagerADT const restrict memoryManager);

#endif /* DUMMY_MEMORY_MANAGER_H */