#include <memoryManager.h>
#include <scheduler.h>
#include <constants.h>
#include <video.h>

#define IS_POWER_OF_2(x) (!((x)&((x)-1)))

typedef enum MemoryStatus { EMPTY, PARTIAL, FULL } MemoryStatus;

#define BLOCK 0x1000

#define KERNEL_MEMORY_LOCATION 0x50000
#define KERNEL_MEMORY_SIZE 0x140000

#define PCB_BLOCK PCBNodeSize
#define SEM_BLOCK semNodeSize
#define IPC_BLOCK IPCBufferSize
#define TIMER_BLOCK timerSize

typedef struct MemNode *MemNodePtr;

typedef struct BuddyMemoryManager{
    MemNodePtr root;
    uint64_t memAllocated;
    uint64_t totalSize;
    uint64_t freeMemory;
} BuddyMemoryManager;

typedef struct MemNode{
    MemNodePtr right;
    MemNodePtr left;
    uint64_t size;
    MemoryStatus status;
} MemNode;

BuddyMemoryManager kernelMemoryManager;
BuddyMemoryManager userlandMemoryManager;

uint64_t PCBNodeSize;
uint64_t semNodeSize;
uint64_t IPCBufferSize;
uint64_t timerSize;

static void initMemory(BuddyMemoryManager *memoryManager, void *const init, uint64_t size);
static void *genericAllocMemory(BuddyMemoryManager *memoryManager, uint64_t size);
static void *genericAllocMemoryRec(BuddyMemoryManager *memoryManager, MemNodePtr node, unsigned int size);
static void setDescendants(BuddyMemoryManager *memoryManager, MemNodePtr node);
static int hasDescendants(MemNodePtr node);
static void endRecursiveCall(BuddyMemoryManager *memoryManager, MemNodePtr node);
static void genericFreeRec(BuddyMemoryManager *memoryManager, MemNodePtr node, void *memoryToFree);
static uint64_t align(uint64_t size);

void createMemoryManager(void *const restrict init, uint64_t size){
    initMemory(&userlandMemoryManager, init, size);
    initMemory(&kernelMemoryManager, KERNEL_MEMORY_LOCATION, KERNEL_MEMORY_SIZE);
    
    PCBNodeSize = getPCBNodeSize();
    semNodeSize = getSemNodeSize();
    IPCBufferSize = getIPCBufferSize();
    timerSize = getTimerSize();
}

static void initMemory(BuddyMemoryManager *memoryManager, void *const init, uint64_t size){
    memoryManager->root = init;
    memoryManager->root->size = size - sizeof(MemNode);
    memoryManager->root->status = EMPTY;
    memoryManager->root->left = NULL;
    memoryManager->root->right = NULL;
}

static void *genericAllocMemory(BuddyMemoryManager *memoryManager, uint64_t size){
    if(size < BLOCK){
        size = BLOCK;
    }
    else if(size > memoryManager->root->size){
        return NULL;
    }
    if(!IS_POWER_OF_2(size)){
        size = align(size);
    }
    
    void * allocAttempt = genericAllocMemoryRec(memoryManager, memoryManager->root, size);
    return allocAttempt;
}

static void *genericAllocMemoryRec(BuddyMemoryManager *memoryManager, MemNodePtr node, unsigned int size){
    if(node == NULL || node->status == FULL){
        return NULL;
    }

    if(node->left != NULL || node->right != NULL){
        void *auxNode = genericAllocMemoryRec(memoryManager, node->left, size);
        if (auxNode == NULL) {
            auxNode = genericAllocMemoryRec(memoryManager, node->right, size);
        }
        endRecursiveCall(memoryManager, node);
        return auxNode;
    } 
    else{
        if (size > node->size) {
            return NULL;
        }
        if ((node->size / 2) >= size) {
            setDescendants(memoryManager, node);
            void *auxNode = genericAllocMemoryRec(memoryManager, node->left, size);
            endRecursiveCall(memoryManager, node);
            return auxNode;
        }
        node->status = FULL;
        memoryManager->memAllocated += node->size + sizeof(MemNode);
        memoryManager->freeMemory -= node->size + sizeof(MemNode);
        return (void *)((uint64_t)node + sizeof(MemNode));
    }
}

static void setDescendants(BuddyMemoryManager *memoryManager, MemNodePtr node){
    uint64_t descendantSize = ((uint64_t)(node->size) / 2);
    node->left = (MemNodePtr)((uint64_t)node + sizeof(MemNode));

    uint64_t limit = (uint64_t)memoryManager->root + memoryManager->root->size + sizeof(MemNode);

    if ((uint64_t) node->left >= limit) {
        return;
    }
    node->left->size = descendantSize - sizeof(MemNode);
    node->left->status = EMPTY;
    memoryManager->memAllocated += sizeof(MemNode);
    memoryManager->freeMemory -= sizeof(MemNode);

    node->right = (MemNodePtr)((uint64_t)node + descendantSize + sizeof(MemNode));
    if ((uint64_t) node->right >= limit) {
        return;
    }
    node->right->size = descendantSize - sizeof(MemNode);
    node->right->status = EMPTY;
    
    memoryManager->memAllocated += sizeof(MemNode);
    memoryManager->freeMemory -= sizeof(MemNode);
}

static int hasDescendants(MemNodePtr node){
    return node->left == NULL || node->right == NULL;
}

static void endRecursiveCall(BuddyMemoryManager *memoryManager, MemNodePtr node){
    if (hasDescendants(node)) {
        node->status = EMPTY;
        memoryManager->freeMemory += node->size;
        memoryManager->memAllocated -= node->size;
        return;
    }
    if (node->left->status == FULL && node->right->status == FULL) {
        node->status = FULL;
    } else if (node->left->status == FULL || node->right->status == FULL
               || node->left->status == PARTIAL || node->right->status == PARTIAL) {
        node->status = PARTIAL;
    } else {
        node->status = EMPTY;
        memoryManager->freeMemory += node->size;
        memoryManager->memAllocated -= node->size;
    }
}

void freeMemory(void const *memoryToFree){
    genericFreeRec(&userlandMemoryManager, userlandMemoryManager.root, memoryToFree);
}

static void genericFreeRec(BuddyMemoryManager *memoryManager, MemNodePtr node, void *memoryToFree){
    if(node == NULL){
        return;
    }
    if (node->left != NULL || node->right != NULL){
        if (node->right != NULL && (uint64_t) node->right + sizeof(MemNode) > (uint64_t) memoryToFree){
            genericFreeRec(memoryManager, node->left, memoryToFree);
        } 
        else{
            genericFreeRec(memoryManager, node->right, memoryToFree);
        }
        endRecursiveCall(memoryManager, node);
        if (node->status == EMPTY){
            node->right = NULL;
            node->left = NULL;
            memoryManager->freeMemory += 2*sizeof(MemNode);
            memoryManager->memAllocated -= 2*sizeof(MemNode);
        }
    } 
    else if (node->status == FULL) {
        if ((void *)((uint64_t)node + sizeof(MemNode)) == memoryToFree) {
            node->status = EMPTY;
            memoryManager->memAllocated -= node->size;
            memoryManager->freeMemory += node->size;
        }
    }
    return;
}
void *allocMemory(const uint64_t memoryToAllocate){
    return genericAllocMemory(&userlandMemoryManager, memoryToAllocate);
}

void *allocPCB(){
    return genericAllocMemory(&kernelMemoryManager, PCB_BLOCK);
}

void *allocSemaphore(){
    return genericAllocMemory(&kernelMemoryManager, SEM_BLOCK);
}

void *allocBuffer(){
    return genericAllocMemory(&kernelMemoryManager, IPC_BLOCK);
}

void *allocTimer(){
    return genericAllocMemory(&kernelMemoryManager, TIMER_BLOCK);
}

void freePCB(void const *blockToFree){
    genericFreeRec(&kernelMemoryManager, kernelMemoryManager.root, blockToFree);
}

void freeSemaphore(void const *semToFree){
    genericFreeRec(&kernelMemoryManager, kernelMemoryManager.root, semToFree);
}

void freeBuffer(void const *bufferToFree){
    genericFreeRec(&kernelMemoryManager, kernelMemoryManager.root, bufferToFree);
}

void freeTimer(void const *timerToFree){
    genericFreeRec(&kernelMemoryManager, kernelMemoryManager.root, timerToFree);
}

void copyBlocks(void const *target, void const *source){
    MemNodePtr targetNode = (MemNodePtr)((uint64_t)targetNode - sizeof(MemNode));
    MemNodePtr sourceNode = (MemNodePtr)((uint64_t)sourceNode - sizeof(MemNode));

    uint8_t *s = (uint8_t *)source;
    uint8_t *t = (uint8_t *)target;
    for(int i=0; i < targetNode->size && i < sourceNode->size ;i++){
        uint8_t aux = s[i];
        t[i] = aux;
    }
    return;
}

void *reAllocMemory(void *memoryToRealloc, uint64_t newSize){
    MemNodePtr memoryToReallocNode = (MemNodePtr)((uint64_t)memoryToRealloc - sizeof(MemNode));

    if(memoryToReallocNode->size >= newSize){
        return memoryToRealloc;
    }
    
    void *newAllocation = allocMemory(newSize);
    copyBlocks(newAllocation, memoryToRealloc);
    return newAllocation;

}

static uint64_t align(uint64_t size){
    size |= size >> 1;
    size |= size >> 2;
    size |= size >> 4;
    size |= size >> 8;
    size |= size >> 16;
    return size + 1;
}

uint64_t getFreeMemoryAmount(){
    return userlandMemoryManager.freeMemory;
}

uint64_t getUsedMemoryAmount(){
    return userlandMemoryManager.memAllocated;
}