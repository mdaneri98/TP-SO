#include <memoryManager.h> 
#include <lib.h>
#include <scheduler.h>

#define TRUE 1
#define FALSE 0
#define BLOCK_SIZE 0x1000
#define PCB_LOCATION 0x50000

typedef struct node{
    struct node *next;
    struct node *prev;
    uint64_t memSize;
    char isFree;
} MMNode;

typedef struct list{
    MMNode *head;
} queue_t;


/* Inicio eliminar */
typedef struct ProcessControlBlockCDT {
    uint32_t id;
    char foreground;
    ProcessState state;
    uint8_t priority;

    // Variables neccesary for computing priority scheduling
    uint8_t quantums;
    uint64_t agingInterval;
    uint64_t currentInterval;
    
    // Each process will have its own buffers for reading and writing (since we don't have a filesystem)
    IPCBuffer readBuffer;
    IPCBuffer writeBuffer;

    // All the information necessary for running the stack of the process
    void *stack;
    void *baseStack;
    uint64_t stackSize;
    void *memoryFromMM;

    IPCBuffer *pdTable[PD_SIZE];
} ProcessControlBlockCDT;

typedef struct pcb_node {
    struct pcb_node *next;
    struct pcb_node *previous;
    ProcessControlBlockCDT pcbEntry;
} PCBNodeCDT;
/* Fin eliminar */

#define PCB_BLOCK sizeof(MMNode) + sizeof(PCBNodeCDT)

typedef struct MemoryManager_t{
    queue_t freeList;
    uint64_t freeMemory;
    uint64_t usedMemory;
    uint64_t totalMemory;
} MemoryManager_t;

MemoryManager_t memoryManager;
MemoryManager_t PCBMemoryManager;

static void initMemory(MemoryManager_t *memoryForMemoryManager, void *const restrict init, uint64_t size);
static void *genericAllocMemory(MemoryManager_t *memoryForMemoryManager, uint64_t blockSize, uint64_t memoryToAllocate);
static void genericFreeMemory(MemoryManager_t *memoryForMemoryManager, void *const restrict memoryToFree);

static void initMemory(MemoryManager_t *memoryForMemoryManager, void *const restrict init, uint64_t size){
    memoryForMemoryManager->freeMemory =  size;
    memoryForMemoryManager->totalMemory = size;
    memoryForMemoryManager->usedMemory = 0;
    // The first element of this freeList is the node containing the root of the memory
    memoryForMemoryManager->freeList.head = (MMNode *)init;
    memoryForMemoryManager->freeList.head->next = (MMNode *)((uint64_t)init + sizeof(MMNode));
    memoryForMemoryManager->freeList.head->prev = NULL;
    memoryForMemoryManager->freeList.head->memSize = 0;
    memoryForMemoryManager->freeList.head->isFree = FALSE;

    // We add a final value to the last node (in this case the whole memory), and the previous memory value (root)
    memoryForMemoryManager->freeList.head->next->next = NULL;
    memoryForMemoryManager->freeList.head->next->prev = memoryForMemoryManager->freeList.head;
    memoryForMemoryManager->freeList.head->next->memSize = size - 2*sizeof(MMNode);
    memoryForMemoryManager->freeList.head->next->isFree = TRUE;
}

static void *genericAllocMemory(MemoryManager_t *memoryForMemoryManager, uint64_t blockSize, uint64_t memoryToAllocate){
    MMNode *currentNode = memoryForMemoryManager->freeList.head->next;
    // We force the memory to have a fixed size of n blocks of 4kBytes
    uint64_t blocksToAllocate = memoryToAllocate + sizeof(MMNode) % blockSize == 0 ? sizeof(MMNode) + memoryToAllocate : ((sizeof(MMNode) + memoryToAllocate)/blockSize)*blockSize + blockSize;
    while(currentNode != NULL){
        if(currentNode->isFree && currentNode->memSize >= blocksToAllocate){
            // If the node is free and has enough memory, we can allocate it
            // We have to check if we can split the node
            if(currentNode->memSize > blocksToAllocate){
                // We can split the node
                MMNode *newNode = (MMNode *)((uint64_t) currentNode + blocksToAllocate);
                newNode->next = currentNode->next;
                newNode->prev = currentNode;
                newNode->memSize = currentNode->memSize - blocksToAllocate;
                newNode->isFree = TRUE;
                currentNode->next = newNode;
                currentNode->memSize = blocksToAllocate - sizeof(MMNode);
            }
            currentNode->isFree = FALSE;
            memoryForMemoryManager->freeMemory -= currentNode->memSize;
            memoryForMemoryManager->usedMemory += currentNode->memSize;
            return (void *)((uint64_t)currentNode + sizeof(MMNode));
        }
        currentNode = currentNode->next;
    }
    return NULL;
}

static void genericFreeMemory(MemoryManager_t *memoryForMemoryManager, void *const memoryToFree){
    MMNode *currentNode = (MMNode *)((uint64_t)memoryToFree - sizeof(MMNode));
    currentNode->isFree = TRUE;
    memoryForMemoryManager->freeMemory += currentNode->memSize;
    memoryForMemoryManager->usedMemory -= currentNode->memSize;
    // We have to check if we can merge the node with the next one
    if(currentNode->next != NULL && currentNode->next->isFree){
        currentNode->memSize += currentNode->next->memSize + sizeof(MMNode);
        currentNode->next = currentNode->next->next;
    }
    // We have to check if we can merge the node with the previous one
    if(currentNode->prev != NULL && currentNode->prev->isFree){
        currentNode->prev->memSize += currentNode->memSize + sizeof(MMNode);
        currentNode->prev->next = currentNode->next;
    }
}

void createMemoryManager(void *const restrict init, uint64_t size) {
	initMemory(&memoryManager, init, size);
    initMemory(&PCBMemoryManager, PCB_LOCATION, 0x140000);
}

void *allocMemory(const uint64_t memoryToAllocate){
    return genericAllocMemory(&memoryManager, BLOCK_SIZE, memoryToAllocate);
}

void *allocPCB(){
    return genericAllocMemory(&PCBMemoryManager, PCB_BLOCK, PCB_BLOCK);
}

void *reAllocMemory(void *const memoryToRealloc, uint64_t newSize){
    MMNode *currentNode = (MMNode *)((uint64_t)memoryToRealloc - sizeof(MMNode));
    // If the new size isn't actually bigger than the amount we given in the first place
    if(currentNode->memSize >= newSize){
        return memoryToRealloc;
    }
    void *newAllocation = allocMemory(newSize);
    if(newAllocation == NULL){
        freeMemory(memoryToRealloc);
        return NULL;
    }
    MMNode *newNode = (MMNode *)((uint64_t)newAllocation - sizeof(MMNode));
    memcpy(newAllocation, memoryToRealloc,  newNode->memSize);
    freeMemory(memoryToRealloc);
    return newAllocation;
}

void freeMemory(void *const memoryToFree){
    genericFreeMemory(&memoryManager, memoryToFree);
}

void freePCB(void *const blockToFree){
    genericFreeMemory(&PCBMemoryManager, blockToFree);
}

uint64_t getFreeMemoryAmount(){
    return memoryManager.freeMemory;
}

uint64_t getUsedMemoryAmount(){
    return memoryManager.usedMemory;
}

void copyBlocks(void *target, void *source){
    MMNode *targetHeader = (MMNode *) ((uint64_t)target - sizeof(MMNode));
    MMNode *sourceHeader = (MMNode *) ((uint64_t)source - sizeof(MMNode));
    int limit = targetHeader->memSize > sourceHeader->memSize ? sourceHeader->memSize : targetHeader->memSize;
    uint8_t *s = (uint8_t *)source;
    uint8_t *t = (uint8_t *)target;
    for(int i=0; i<limit ;i++){
        t[i] = s[i];
    }
}

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