#include <dummyMemoryManager.h>


typedef struct MemoryManagerCDT {
	uint8_t *nextAddress;
    uint8_t *limit;
} MemoryManagerCDT;

MemoryManagerADT createMemoryManager(void *const restrict memoryForMemoryManager, void *const restrict managedMemory, uint64_t size) {
    MemoryManagerADT memoryManager = (MemoryManagerADT) memoryForMemoryManager;
	memoryManager->nextAddress = (uint8_t *)managedMemory;
    memoryManager->limit = (uint8_t *)((uint64_t)managedMemory + size);

	return memoryManager;
}

void *allocMemory(MemoryManagerADT const memoryManager, const uint64_t memoryToAllocate) {
	uint8_t *allocation = NULL;
    if(((uint64_t) memoryManager->nextAddress + memoryToAllocate) < (uint64_t) memoryManager->limit){
        allocation = memoryManager->nextAddress;
    }
    else{
        return (void *)allocation;
    }

	memoryManager->nextAddress += memoryToAllocate;

	return (void *) allocation;
}

void freeMemory(MemoryManagerADT const restrict memoryManager, void *const restrict memoryToFree, const uint64_t memorySize){
    uint8_t *aux = (uint8_t* ) ((uint64_t) memoryToFree + memorySize);
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