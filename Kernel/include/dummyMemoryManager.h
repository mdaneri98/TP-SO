#ifndef DUMMY_MEMORY_MANAGER_H
#define DUMMY_MEMORY_MANAGER

#include <stdint.h>

#define NULL (void *)0

typedef struct MemoryManagerCDT *MemoryManagerADT;

MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory, void *const restrict limit);

void *allocMemory(MemoryManagerADT const restrict memoryManager, const uint64_t memoryToAllocate);

void freeMemory(MemoryManagerADT const restrict memoryManager, void *const restrict memoryToFree, const uint64_t memorySize);

#endif /* DUMMY_MEMORY_MANAGER_H */