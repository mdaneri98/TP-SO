#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>
#include <keyboard.h>
#include <syscallDispatcher.h>
#include <libasm.h>
#include <video.h>

#define DEFAULT_PROCESS_STACK_SIZE 0x1500
#define QUANTUM_SIZE 55

#define INIT_ID 1
#define IDLE_ID 2

#define TRUE 1
#define FALSE 0

/* Structures */
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
typedef struct queue {
    PCBNodeCDT *head;
    uint8_t defaultQuantums;
} queue_t;

/* Prototypes */
uint32_t unusedID();
char exists(uint32_t pid);
int add(ProcessControlBlockCDT newEntry);
static void closePDs(pid);
static void removeReferences(IPCBuffer *pdBuffer, uint32_t pid);
static void insertInQueue(PCBNodeCDT *node);
static void insertInBlockedQueue(PCBNodeCDT *node);
static void checkBlocked();
static void checkExited();
static void next();

/* Global Variables */
queue_t level0Queue = {NULL, NULL, 1};
queue_t level1Queue = {NULL, NULL, 2};
queue_t level2Queue = {NULL, NULL, 4};
queue_t level3Queue = {NULL, NULL, 8};
queue_t level4Queue = {NULL, NULL, 16};
queue_t level5Queue = {NULL, NULL, 32};
queue_t blockedList = {NULL, NULL, 0};

PCBNodeCDT *current = NULL;
PCBNodeCDT *idle = NULL;

queue_t *multipleQueues[] = {&level0Queue, &level1Queue, &level2Queue, 
                                &level3Queue, &level4Queue, &level5Queue, &blockedList};

// CPU speed in MHz
uint32_t cpuSpeed;

void *scheduler(void *rsp) {
    // When the system starts up, the current field is always NULL
    if(current == NULL){
        current = level0Queue.head;
        level0Queue.head = NULL;
        return current->pcbEntry.stack;
    }

    // Backup of the caller stack
    current->pcbEntry.stack = rsp;
    uint64_t cicles = readTimeStampCounter();
    current->pcbEntry.agingInterval /= 2;
    current->pcbEntry.agingInterval += (cicles - current->pcbEntry.currentInterval)/cpuSpeed;

    checkBlocked();
    checkExited();

    // If this function was called by a process, we need to check its state
    if (current->pcbEntry.state == EXITED) {
        PCBNodeCDT *aux = current;
        
        // We move to the next process that needs execution
        next();
        current->pcbEntry.currentInterval = readTimeStampCounter()/cpuSpeed;
        
        // We remove the EXITED process from the list of processes
        remove(aux);

        return current->pcbEntry.stack;
    }

    /* 
        If the process was BLOCKED or RUNNING, and scheduler function started, we do the same thing. 
        Get the next process with a READY state and run it.
    */
    current->pcbEntry.quantums--;
    if(current->pcbEntry.quantums < 1 || current->pcbEntry.state == BLOCKED){
        next();
    }
    current->pcbEntry.currentInterval = readTimeStampCounter()/cpuSpeed;

    return current->pcbEntry.stack;
}

static void checkBlocked(){
    PCBNodeCDT *aux = multipleQueues[6]->head;
    while(aux != NULL){
        if(aux->pcbEntry.state != BLOCKED){
            PCBNodeCDT *toInsert = aux;
            aux = aux->next;
            insertInQueue(toInsert);
            if(toInsert == multipleQueues[6]->head){
                multipleQueues[6]->head = toInsert->next;
                toInsert->next->previous = NULL;
            } else{
                toInsert->previous->next = toInsert->next;
                if(toInsert->next != NULL){
                    toInsert->next->previous = toInsert->previous;
                }
            }
        } else{
            aux = aux->next;
        }
    }
}

static void checkExited(){
    PCBNodeCDT *toRemove;
    for(int i=0; i<7 ;i++){
        toRemove = multipleQueues[i]->head;
        while(toRemove != NULL){
            if(toRemove->pcbEntry.state == EXITED){
                if(multipleQueues[i]->head == toRemove){
                    multipleQueues[i]->head = toRemove->next;
                } else{
                    toRemove->previous->next = toRemove->next;
                }
                toRemove->next->previous = toRemove->previous;
                remove(toRemove);
            }
            toRemove = toRemove->next;
        }
    }
}

void createInit() {
    void *memoryForInit = allocMemory(DEFAULT_PROCESS_STACK_SIZE);
    void *initStack = (void *)((uint64_t) memoryForInit + DEFAULT_PROCESS_STACK_SIZE);
    PCBNodeCDT *initNode = allocPCB();
    void *memoryForIdle = allocMemory(DEFAULT_PROCESS_STACK_SIZE);
    void *idleStack = (void *)((uint64_t) memoryForIdle + DEFAULT_PROCESS_STACK_SIZE);
    PCBNodeCDT *idleNode = allocPCB();
    
    initNode->previous = NULL;
    initNode->next = NULL;
    idleNode->previous = NULL;
    idleNode->next = NULL;

    initNode->pcbEntry.pdTable[0] = getSTDIN();
    initNode->pcbEntry.pdTable[0]->references[0] = &initNode->pcbEntry;

    initNode->pcbEntry.pdTable[1] = getSTDOUT();
    initNode->pcbEntry.pdTable[1]->references[0] = &initNode->pcbEntry;

    initNode->pcbEntry.pdTable[2] = getSTDERR();
    initNode->pcbEntry.pdTable[2]->references[0] = &initNode->pcbEntry;
    
    for(int i=3; i<PD_SIZE ;i++){
        initNode->pcbEntry.pdTable[i] = NULL;
    }
    initNode->pcbEntry.stackSize = DEFAULT_PROCESS_STACK_SIZE;
    initNode->pcbEntry.baseStack = initStack;
    initNode->pcbEntry.stack = createInitStack(initStack);
    initNode->pcbEntry.memoryFromMM = memoryForInit;
    initNode->pcbEntry.id = INIT_ID;
    initNode->pcbEntry.foreground = 1;
    initNode->pcbEntry.state = READY;
    initNode->pcbEntry.currentInterval = 0;
    initNode->pcbEntry.agingInterval = 0;
    initNode->pcbEntry.quantums = 1;
    initNode->pcbEntry.priority = 0;
    level0Queue.head = initNode;

    idleNode->pcbEntry.stackSize = DEFAULT_PROCESS_STACK_SIZE;
    idleNode->pcbEntry.baseStack = idleStack;
    idleNode->pcbEntry.stack = createWaiterStack(idleStack);
    idleNode->pcbEntry.memoryFromMM = memoryForIdle;
    idleNode->pcbEntry.id=IDLE_ID;
    idleNode->pcbEntry.foreground = 1;
    idleNode->pcbEntry.state = READY;
    idleNode->pcbEntry.currentInterval = 0;
    idleNode->pcbEntry.agingInterval = 0;
    idleNode->pcbEntry.quantums = 1;
    idle = idleNode;
    
    current = NULL;
    
    cpuSpeed = (uint32_t) getCPUSpeed();
    // We pass the CPU speed from MHz to KHz (it makes the calculus of intervals in ms easier)
    cpuSpeed *= 1000;
}

/*
    The next process will be:
        1. The first process that is on READY state
        2. The idle process that halts until any other process is on READY state
*/
static void next(){
    if(current->pcbEntry.id == idle->pcbEntry.id){
            current->pcbEntry.state = READY;
            current = NULL;
    } else if(current->pcbEntry.state == BLOCKED){
        insertInBlockedQueue(current);
    } else if(current->pcbEntry.state == RUNNING){
        current->pcbEntry.state = READY;
        multipleQueues[current->pcbEntry.priority]->head = current->next;
        insertInQueue(current);
    }

    PCBNodeCDT *next;
    for(int i=0; i<6 ;i++){
        next = multipleQueues[i]->head;
        if(next != NULL){
            multipleQueues[i]->head = next->next;
            break;
        }
    }
    current = next;
    // If the last node isn't READY, we set the idle
    if (current == NULL) {
        current = idle;
        current->pcbEntry.quantums = 1;
        current->pcbEntry.state = RUNNING;
        return current->pcbEntry;
    }

    current->next = NULL;
    current->previous = NULL;
    current->pcbEntry.state = RUNNING;
    current->pcbEntry.quantums = 1;
}

static void insertInQueue(PCBNodeCDT *node){
    uint64_t agingTime = node->pcbEntry.agingInterval;
    int i;
    if(agingTime < QUANTUM_SIZE){
        i = 0;
    } else if(agingTime < QUANTUM_SIZE*2){
        i = 1;
    } else if(agingTime < QUANTUM_SIZE*4){
        i = 2;
    } else if(agingTime < QUANTUM_SIZE*8){
        i = 3;
    } else if(agingTime < QUANTUM_SIZE*16){
        i = 4;
    } else {
        i = 5;
    }
    PCBNodeCDT *aux = multipleQueues[i]->head;
    if(aux == NULL){
        multipleQueues[i]->head = node;
    } else{
        while(aux->next != NULL){
            aux = aux->next;
        }
        aux->next = node;
    }
    node->next = NULL;
    node->pcbEntry.quantums = multipleQueues[i]->defaultQuantums;
    node->pcbEntry.priority = i;
}

static void insertInBlockedQueue(PCBNodeCDT *node){
    PCBNodeCDT *first = multipleQueues[6]->head;
    while(first != NULL && first->next != NULL){
        first = first->next;
    }
    if(first == NULL){
        multipleQueues[6]->head = node;
        node->previous = NULL;
    } else{
        first->next = node;
        node->previous = first;
    }
    node->next = NULL;
}

int remove(PCBNodeCDT *toRemove) {
    closePDs(toRemove->pcbEntry.id);
    freeMemory(toRemove->pcbEntry.memoryFromMM);
    freePCB(toRemove);

    return toRemove->pcbEntry.id;
}

static void closePDs(uint32_t pid){
    if(pid == 0){
        // pid 0 belongs to standart input/output, can't close
        return;
    }
    PCBNodeCDT *node = level0Queue.head;
    while(node != NULL){
        if(node->pcbEntry.id == pid){
            continue;
        }
        for(int i=0; i<PD_SIZE ;i++){
            if(node->pcbEntry.pdTable[i] != NULL && node->pcbEntry.pdTable[i]->buffId == pid){
                removeReferences(node->pcbEntry.pdTable[i], pid);
                node->pcbEntry.pdTable[i] = NULL;
            }
        }
        node = node->next;
    }
}

static void removeReferences(IPCBuffer *pdBuffer, uint32_t pid){
    for(int i=0; i<PD_SIZE ;i++){
        if(pdBuffer->references[i] != NULL && pdBuffer->references[i]->id == pid){
            pdBuffer->references[i] == NULL;
        }
    }
}

// It always has to be at least one process on the list
uint32_t unusedID() {
    uint32_t max;
    if(current != NULL){
        max = current->pcbEntry.id;
    }
    PCBNodeCDT *node;
    for(int i=0; i<7 ;i++){
        node = multipleQueues[i]->head;
        while(node != NULL){
            max = node->pcbEntry.id > max ? node->pcbEntry.id : max;
            node = node->next;
        }
    }

    return max + 1;
}

char exists(uint32_t pid) {
    PCBNodeCDT *aux;
    
    for(int i=0; i<7 ;i++){
        aux = multipleQueues[i]->head;
        while(aux != NULL){
            if(aux->pcbEntry.id == pid){
                return TRUE;
            }
        }
    }
    return FALSE;
}

int sysFork() {
    PCBNodeCDT *newNode = allocPCB();
    if(newNode == NULL){
        return -1;
    }
    PCBNodeCDT *currentNode = current;
    void *memoryForFork = allocMemory(currentNode->pcbEntry.stackSize);
    void *newStack = (void *)((uint64_t) memoryForFork + currentNode->pcbEntry.stackSize);
    if(newStack == NULL){
        return -1;
    }
    newNode->pcbEntry.stackSize = currentNode->pcbEntry.stackSize;
    newNode->pcbEntry.memoryFromMM = memoryForFork;
    newNode->pcbEntry.baseStack = newStack;
    copyState(&newStack, currentNode->pcbEntry.stack);
    newNode->pcbEntry.stack = newStack;

    newNode->pcbEntry.foreground = currentNode->pcbEntry.foreground;
    newNode->pcbEntry.state = currentNode->pcbEntry.state;
    newNode->pcbEntry.priority = currentNode->pcbEntry.priority;

    newNode->pcbEntry.quantums = currentNode->pcbEntry.quantums;
    newNode->pcbEntry.agingInterval = currentNode->pcbEntry.agingInterval;
    newNode->pcbEntry.currentInterval = currentNode->pcbEntry.currentInterval;

    for(int i=0; i<PD_SIZE ;i++){
        newNode->pcbEntry.pdTable[i] = currentNode->pcbEntry.pdTable[i];
        newNode->pcbEntry.readBuffer.references[i] = currentNode->pcbEntry.readBuffer.references[i];
        newNode->pcbEntry.writeBuffer.references[i] = currentNode->pcbEntry.writeBuffer.references[i];
    }
    
    for(int i=0; i<PD_BUFF_SIZE ;i++){
        newNode->pcbEntry.writeBuffer.buffer[i] = currentNode->pcbEntry.writeBuffer.buffer[i];
        newNode->pcbEntry.readBuffer.buffer[i] = currentNode->pcbEntry.readBuffer.buffer[i];
    }

    // Should be other id
    uint64_t parentId = currentNode->pcbEntry.id;
    newNode->pcbEntry.id = unusedID();

    newNode->pcbEntry.writeBuffer.buffId = newNode->pcbEntry.id;
    newNode->pcbEntry.readBuffer.buffId = newNode->pcbEntry.id;

    insertInQueue(newNode);

    // If we are in the parent process
    if(currentNode->pcbEntry.id == parentId){
        return newNode->pcbEntry.id;
    }

    // We are in the child process
    return 0;
}

int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp){
    ProcessControlBlockCDT *currentProcess = &current->pcbEntry;

    // IT ONLY SETUPS THE PROCESS CORRECTLY IF ITS CALLED BY THE SYSCALL, U NEED TO SET THE STACK
    //BEFORE USING IT WITH INIT OR ANY FUNCTIONALITY THAT USES THIS FUNCTION WITHOUT USING THE SYSCALL
    if(setProcess(process, argc, argv, rsp) == -1){
        return -1;
    }
    currentProcess->stack = rsp;
    currentProcess->currentInterval = 0;
    currentProcess->agingInterval = 0;
    currentProcess->priority = 0;
    currentProcess->quantums = 1;
    for(int i=0; i<PD_BUFF_SIZE ;i++){
        currentProcess->readBuffer.buffer[i] = '\0';
        currentProcess->writeBuffer.buffer[i] = '\0';
    }
    return 1;
}

ProcessControlBlockADT getEntry(uint32_t pid) {
    if (!exists(pid)) {
        return NULL;
    }

    PCBNodeCDT *toReturn;
    for(int i=0; i<7 ;i++){
        toReturn = multipleQueues[i]->head;
        while(toReturn != NULL){
            if(toReturn->pcbEntry.id == pid){
                return &toReturn->pcbEntry;
            }
            toReturn = toReturn->next;
        }
    }

    return NULL;
}

PCBNodeADT getCurrentProcess(){
    return current;
}

ProcessControlBlockADT getCurrentProcessEntry(){
    return &current->pcbEntry;
}

IPCBuffer *getPDEntry(ProcessControlBlockADT entry, uint32_t pd){
    return entry->pdTable[pd];
}

void setProcessState(ProcessControlBlockADT process, ProcessState state){
    process->state = state;
}