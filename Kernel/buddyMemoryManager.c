#include "buddyMemoryManager.h"
#include <stdio.h>

#define FREE 0
#define SPLITTED 1
#define OCCUPIED 2
#define FLOOR 4096

typedef struct block_t{
    struct block_t *parent;
    struct block_t *rBlock;
    struct block_t *lBlock;
    uint64_t size;
    uint8_t state;
} block_t;

typedef struct BuddyMemoryManager_t{
    uint64_t totalMemory;
    block_t *mainBlock;
} BuddyMemoryManagerCDT;

BuddyMemoryManagerCDT memoryManager;

void setBlocks(block_t * block);
void setFree(block_t * currentBlock);
uint64_t getSize(block_t * currentBlock, int state);
void *BSMem(block_t *currentBlock, uint64_t memoryToAllocate);
void freeParentBlock(block_t * blockToFree);


void createBuddyMemoryManager(void *const restrict init, uint64_t size) {
    
    uint64_t powerOfTwoSize = size;
   
    memoryManager.totalMemory = powerOfTwoSize;
    memoryManager.mainBlock = (block_t*) init;
    memoryManager.mainBlock->state = FREE;
    memoryManager.mainBlock->size = powerOfTwoSize;
    memoryManager.mainBlock->rBlock = NULL;
    memoryManager.mainBlock->lBlock = NULL;
    memoryManager.mainBlock->parent = NULL;
}

void *allocMemory(uint64_t memoryToAllocate){
    block_t *currentBlock = memoryManager.mainBlock;
    return BSMem(currentBlock, memoryToAllocate);
}

void *BSMem(block_t *currentBlock, uint64_t memoryToAllocate){
    uint64_t blockSize = currentBlock->size;
    
    if((blockSize - sizeof(block_t))/2 < memoryToAllocate && currentBlock->state == FREE){
        currentBlock->state = OCCUPIED;
        block_t *parent = currentBlock->parent;
        // if(parent->lBlock != NULL && parent->lBlock->state == OCCUPIED && parent->rBlock != NULL && parent->rBlock->state == OCCUPIED){
        //     parent->state = OCCUPIED;
        // }
        return (void*)((uint64_t)currentBlock + sizeof(block_t));
    }
    if(blockSize < FLOOR){
        return NULL;
    }
    
    if(currentBlock->lBlock != NULL && currentBlock->lBlock->state != OCCUPIED){
        return BSMem(currentBlock->lBlock, memoryToAllocate);
    } else if(currentBlock->rBlock != NULL && currentBlock->rBlock->state != OCCUPIED){
        return BSMem(currentBlock->rBlock, memoryToAllocate);
    } else if(currentBlock->lBlock == NULL){
        currentBlock->lBlock = (block_t *)((uint64_t) currentBlock + sizeof(block_t));
        currentBlock->lBlock->parent = currentBlock;
        currentBlock->lBlock->lBlock = NULL;
        currentBlock->lBlock->rBlock = NULL;
        currentBlock->lBlock->size = (blockSize - sizeof(block_t))/2;
        currentBlock->lBlock->state = FREE;
        currentBlock->state = SPLITTED;
        return BSMem(currentBlock->lBlock, memoryToAllocate);
    } else if(currentBlock->rBlock == NULL){
        currentBlock->rBlock = (block_t *)((uint64_t) currentBlock + sizeof(block_t) + blockSize/2 + 1);
        currentBlock->rBlock->parent = currentBlock;
        currentBlock->rBlock->lBlock = NULL;
        currentBlock->rBlock->rBlock = NULL;
        currentBlock->rBlock->size = (blockSize - sizeof(block_t))/2 + 1;
        currentBlock->rBlock->state = FREE;
        currentBlock->state = SPLITTED;
        return BSMem(currentBlock->rBlock, memoryToAllocate);
    } else {
        return NULL;
    }
}

void freeMemory(void *const restrict memoryToFree){
    block_t* blockToFree = (block_t*)(((uint64_t)memoryToFree) - sizeof(block_t));
    blockToFree->state = FREE;

    freeParentBlock(blockToFree);
    
    return;
}

void freeParentBlock(block_t * blockToFree){
    block_t* currentBlock = blockToFree->parent;
    while(currentBlock->size <= memoryManager.mainBlock->size && currentBlock->state == OCCUPIED){
        if(currentBlock->lBlock->state == FREE && currentBlock->rBlock->state == FREE){
            currentBlock->state = FREE;
        }
        currentBlock = currentBlock->parent;
    }
    return;
}

void*reallocMemory(void *const restrict memoryToRealloc, uint64_t newSize){
    block_t*currentBlock = (block_t *)((uint64_t)memoryToRealloc - sizeof(block_t));
    if(currentBlock->size > newSize){
        return memoryToRealloc;
    }
    void*newMemory = allocMemory(newSize);
    if(newMemory == NULL){
        freeMemory(memoryToRealloc);
        return NULL;
    }
    block_t *newBlock = (block_t*)((uint64_t)newMemory - sizeof(block_t));

    memcpy(newMemory, memoryToRealloc,  newBlock->size > currentBlock->size ? newBlock->size : currentBlock->size);

    freeMemory(memoryToRealloc);
    return newMemory;
}



// uint64_t getFreeMemoryAmount(){
//     uint64_t freeMemoryAmount = getSize(memoryManager.mainBlock, TRUE);
//     return freeMemoryAmount;
// }

// uint64_t getUsedMemoryAmount(){
//     uint64_t usedMemoryAmount = getSize(memoryManager.mainBlock, FALSE);
//     return usedMemoryAmount;
// }


uint64_t getPowerOfTwo(uint64_t number){
    if (number == 1) { return 0; }
    int power = 0;
    int powerOfTwoSize = 1;
    while(number != 1){
        if(number%2 != 0){
            number++;
        } 
        number = number/2;
        power++;
    }
    for(int i = 0; i<power; i++){
        powerOfTwoSize = powerOfTwoSize*2;
    }
    return powerOfTwoSize;
}

uint64_t getSize(block_t * currentBlock, int state){
    uint16_t size = 0;
    if(state ? !currentBlock->state : currentBlock->state){
        return 0;
    } else if (currentBlock->rBlock == NULL && currentBlock->lBlock == NULL){
        return currentBlock->size;
    }
    size += getSize(currentBlock->rBlock, state);
    size += getSize(currentBlock->lBlock, state);
    return size;
}
