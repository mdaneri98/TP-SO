// ALL THE MEMORY MANAGEMENT IS SUPPOSED TO BE MANAGED LOCALLY IN THIS SOURCE CODE
#include <freeListMemoryManager.h> 

#define TRUE 1
#define FALSE 0

typedef struct node{
    struct node *next;
    struct node *prev;
    uint64_t memSize;
    char isFree;
} node_t;

typedef struct list{
    node_t head;
} list_t;


typedef struct DefaultMemoryManager_t{
    list_t freeList;
    uint64_t freeMemory;
    uint64_t usedMemory;
    uint64_t totalMemory;
} DefaultMemoryManagerCDT;

DefaultMemoryManagerCDT memoryManager;
list_t freeList;


void createDefaultMemoryManager(void *const restrict init, uint64_t size) {
	memoryManager.freeList = freeList;
    memoryManager.freeMemory =  size;
    memoryManager.totalMemory = size;
    memoryManager.usedMemory = 0;
    // The first element of this freeList is the node containing the root of the memory
    memoryManager.freeList.head.next = (node_t *)((uint64_t)init + sizeof(node_t));
    memoryManager.freeList.head.prev = (node_t *)((uint64_t)init + sizeof(node_t));
    memoryManager.freeList.head.memSize = 0;
    memoryManager.freeList.head.isFree = FALSE;
}

void *allocMemory(const uint64_t memoryToAllocate){
    node_t *currentNode = memoryManager.freeList.head.next;
    while(currentNode != NULL){
        if(currentNode->isFree && currentNode->memSize >= memoryToAllocate){
            // If the node is free and has enough memory, we can allocate it
            // We have to check if we can split the node
            if(currentNode->memSize > memoryToAllocate + sizeof(node_t)){
                // We can split the node
                node_t *newNode = (node_t *)((uint64_t)currentNode + sizeof(node_t) + memoryToAllocate);
                newNode->next = currentNode->next;
                newNode->prev = currentNode;
                newNode->memSize = currentNode->memSize - memoryToAllocate - sizeof(node_t);
                newNode->isFree = TRUE;
                currentNode->next = newNode;
                currentNode->memSize = memoryToAllocate;
            }
            currentNode->isFree = FALSE;
            memoryManager.freeMemory -= currentNode->memSize;
            memoryManager.usedMemory += currentNode->memSize;
            return (void *)((uint64_t)currentNode + sizeof(node_t));
        }
        currentNode = currentNode->next;
    }
    return NULL;
}

void freeMemory(void *const restrict memoryToFree){
    node_t *currentNode = (node_t *)((uint64_t)memoryToFree - sizeof(node_t));
    currentNode->isFree = TRUE;
    memoryManager.freeMemory += currentNode->memSize;
    memoryManager.usedMemory -= currentNode->memSize;
    // We have to check if we can merge the node with the next one
    if(currentNode->next != NULL && currentNode->next->isFree){
        currentNode->memSize += currentNode->next->memSize + sizeof(node_t);
        currentNode->next = currentNode->next->next;
    }
    // We have to check if we can merge the node with the previous one
    if(currentNode->prev != NULL && currentNode->prev->isFree){
        currentNode->prev->memSize += currentNode->memSize + sizeof(node_t);
        currentNode->prev->next = currentNode->next;
    }
}

uint64_t getFreeMemoryAmount(){
    return memoryManager.freeMemory;
}

uint64_t getUsedMemoryAmount(){
    return memoryManager.usedMemory;
}