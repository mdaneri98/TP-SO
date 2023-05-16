#include <dummyMemoryManager.h>


typedef struct MemoryManagerCDT {
	char *nextAddress;
    char *limit;
} MemoryManagerCDT;

MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory, void *const restrict limit) {
	MemoryManagerADT memoryManager = (MemoryManagerADT) memoryForMemoryManager;
	memoryManager->nextAddress = managedMemory;
    memoryManager->limit = limit;

	return memoryManager;
}

void *allocMemory(MemoryManagerADT const memoryManager, const uint64_t memoryToAllocate) {
	char *allocation = NULL;
    if(((uint64_t) memoryManager->nextAddress + memoryToAllocate) < (uint64_t) memoryManager->limit)
         allocation = memoryManager->nextAddress;
    else
        return (void *)allocation;

	memoryManager->nextAddress += memoryToAllocate;

	return (void *) allocation;
}

void freeMemory(MemoryManagerADT const restrict memoryManager, void *const restrict memoryToFree, const uint64_t memorySize){
    char *aux = (char* ) ((uint64_t) memoryToFree + memorySize);
    while(aux < memoryManager->nextAddress){
        *(aux - memorySize) = *aux;
        aux++;
    }
    memoryManager->nextAddress -= memorySize;
}

uint64_t getFreeMemoryAmount(MemoryManagerADT const restrict memoryManager){
    return (uint64_t) memoryManager->limit - (uint64_t) memoryManager->nextAddress;
}

uint64_t getUsedMemoryAmount(MemoryManagerADT const restrict memoryManager){
    return (uint64_t) memoryManager->nextAddress - (uint64_t) memoryManager;
}