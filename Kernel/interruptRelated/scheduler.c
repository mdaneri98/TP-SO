// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>
#include <keyboard.h>
#include <syscallsDispatcher.h>
#include <libasm.h>
#include <lib.h>
#include <video.h>
#include <ps.h>
#include <pipe.h>
#include <timer.h>
#include <constants.h>

#define DEFAULT_PROCESS_STACK_SIZE 0x4000
#define QUANTUM_SIZE 55
#define MEMORY_ALIGN 8

#define INIT_ID 1
#define IDLE_ID 2

typedef struct PCBNodeCDT *PCBNodeADT;

/* Structures */
typedef struct ProcessControlBlockCDT {
    uint32_t id;
    uint8_t foreground;
    ProcessState state;
    int manualBlocked;
    uint8_t priority;

    // Variables neccesary for computing priority scheduling
    int quantums;
    uint64_t agingInterval;
    uint64_t counterInit;

    // All related to process hierarchy
    uint32_t parentId;
    uint32_t childsIds[PD_SIZE];

    // All the information necessary for running the stack of the process
    void *stack;
    void *baseStack;
    uint64_t stackSize;
    
    void *memoryFromMM;
    struct ProcessAllocations *firstAlloc;

    IPCBufferADT pdTable[PD_SIZE];
} ProcessControlBlockCDT;

typedef struct ProcessAllocations{
    void *allocation;
    struct ProcessAllocations *next;
} ProcessAllocations;

typedef struct PCBNodeCDT {
    PCBNodeADT next;
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
static void closePDs(ProcessControlBlockADT process);
static void insertInQueue(PCBNodeCDT *node);
static void insertInBlockedQueue(PCBNodeCDT *node);
static void checkChilds(ProcessControlBlockADT pcbEntry);
static void setParentReady(ProcessControlBlockADT pcbEntry);
static void removeReferences(IPCBufferADT pdBuff, uint32_t pid);
static void checkBlocked();
static void checkExited();
static void nextProcess();
static int deleteProcess(PCBNodeCDT *toDelete);
static PCBNodeADT removeFromQueue(uint32_t pid);
static void insertLast(PCBNodeADT process);
static void freeAllocations(ProcessControlBlockADT process);
static void setForegroundProcess(ProcessControlBlockADT process);

/* Global Variables */
queue_t level0Queue = {NULL, 1};
queue_t level1Queue = {NULL, 2};
queue_t level2Queue = {NULL, 3};
queue_t level3Queue = {NULL, 4};
queue_t level4Queue = {NULL, 5};
queue_t level5Queue = {NULL, 6};
queue_t blockedList = {NULL, 0};

PCBNodeADT current = NULL;
PCBNodeADT idle = NULL;
ProcessControlBlockADT foreground = NULL;

queue_t *multipleQueues[] = {&level0Queue, &level1Queue, &level2Queue, 
                                &level3Queue, &level4Queue, &level5Queue, &blockedList};

// Processor's Time Stamp Counter Frequency
uint64_t TSCFrequency;

void *scheduler(void *rsp) {
    // When the system starts up, the current field is always NULL
    if(current == NULL){
        current = level0Queue.head;
        level0Queue.head = NULL;
        return current->pcbEntry.stack; /* current es el proceso init */
    }

    // Backup of the caller stack
    current->pcbEntry.stack = rsp;
    uint64_t counterFinish = _readTimeStampCounter();
    current->pcbEntry.agingInterval /= 2;

    // We check if it reached the value overflow
    uint64_t newInterval = (counterFinish - current->pcbEntry.counterInit)/TSCFrequency;
    current->pcbEntry.agingInterval += newInterval;

    updateTimers(counterFinish/TSCFrequency);
    checkBlocked();
    checkExited();

    /* 
        If the process was BLOCKED or RUNNING, and scheduler function started, we do the same thing. 
        Get the nextProcess process with a READY state and run it.
    */
    current->pcbEntry.quantums--;
    if(current->pcbEntry.quantums < 1 || current->pcbEntry.state == BLOCKED || current->pcbEntry.state == YIELDED){
        nextProcess();
    }
    current->pcbEntry.counterInit = _readTimeStampCounter();

    return current->pcbEntry.stack;
}

static void checkBlocked(){
    PCBNodeADT current = multipleQueues[6]->head;
    PCBNodeADT previous = NULL;
    while(current != NULL){
        if(current->pcbEntry.state != BLOCKED){
            PCBNodeCDT *toInsert = current;
            current = current->next;
            if(previous == NULL){
                multipleQueues[6]->head = current;
            } else{
                previous->next = current;
            }
            insertInQueue(toInsert);
        } else{
            previous = current;
            current = current->next;
        }
    }
}

static void checkExited(){
    PCBNodeADT auxCurrent;
    PCBNodeADT previous;
    for(int i=0; i<7 ;i++){
        auxCurrent = multipleQueues[i]->head;
        previous = NULL;
        while(auxCurrent != NULL){
            if(auxCurrent->pcbEntry.state == EXITED){
                PCBNodeADT toRemove = auxCurrent;
                auxCurrent = auxCurrent->next;
                if(previous == NULL){
                    multipleQueues[i]->head = auxCurrent;
                } else{
                    previous->next = auxCurrent;
                }
                deleteProcess(toRemove);

            } else{
                previous = auxCurrent;
                auxCurrent = auxCurrent->next;
            }
        }
    }
    if(current->pcbEntry.state == EXITED){
        deleteProcess(current);
        checkBlocked();
        nextProcess();
    }
}

void createInit() {
    void *memoryForInit = allocMemory(DEFAULT_PROCESS_STACK_SIZE);
    void *initStack = (void *)((uint64_t) memoryForInit + DEFAULT_PROCESS_STACK_SIZE);
    initStack = (void *)((uint64_t)initStack - (uint64_t)initStack % MEMORY_ALIGN);
    PCBNodeCDT *initNode = allocPCB();
    void *memoryForIdle = allocMemory(DEFAULT_PROCESS_STACK_SIZE);
    void *idleStack = (void *)((uint64_t) memoryForIdle + DEFAULT_PROCESS_STACK_SIZE);
    idleStack = (void *)((uint64_t)idleStack - (uint64_t)idleStack % MEMORY_ALIGN);
    PCBNodeCDT *idleNode = allocPCB();

    initNode->pcbEntry.firstAlloc = NULL;
    idleNode->pcbEntry.firstAlloc = NULL;
    
    initNode->next = NULL;
    idleNode->next = NULL;

    initStandardBuffers();

    IPCBufferADT stdin = getSTDIN();
    IPCBufferADT stdout = getSTDOUT();
    IPCBufferADT stderr = getSTDERR();

    initNode->pcbEntry.pdTable[0] = stdin;
    setReferenceByIndex(initNode->pcbEntry.pdTable[0], &initNode->pcbEntry, 0);
    initNode->pcbEntry.childsIds[0] = 0;

    initNode->pcbEntry.pdTable[1] = stdout;
    setReferenceByIndex(initNode->pcbEntry.pdTable[1], &initNode->pcbEntry, 1);
    initNode->pcbEntry.childsIds[1] = 0;

    initNode->pcbEntry.pdTable[2] = stderr;
    setReferenceByIndex(initNode->pcbEntry.pdTable[2], &initNode->pcbEntry, 2);
    initNode->pcbEntry.childsIds[2] = 0;
    initNode->pcbEntry.parentId = 0;
    
    for(int i=3; i<PD_SIZE ;i++){
        initNode->pcbEntry.pdTable[i] = NULL;
        initNode->pcbEntry.childsIds[i] = 0;
    }
    initNode->pcbEntry.stackSize = initStack - memoryForInit;
    initNode->pcbEntry.baseStack = initStack;
    initNode->pcbEntry.stack = _createInitStack(initStack);
    initNode->pcbEntry.memoryFromMM = memoryForInit;
    initNode->pcbEntry.id = INIT_ID;
    initNode->pcbEntry.foreground = 1;
    initNode->pcbEntry.state = READY;
    initNode->pcbEntry.counterInit = 0;
    initNode->pcbEntry.agingInterval = 0;
    initNode->pcbEntry.quantums = 1;
    initNode->pcbEntry.priority = 0;
    initNode->pcbEntry.foreground = TRUE;
    level0Queue.head = initNode;

    foreground = &initNode->pcbEntry;

    idleNode->pcbEntry.stackSize = idleStack - memoryForIdle;
    idleNode->pcbEntry.baseStack = idleStack;
    idleNode->pcbEntry.stack = _createIdleStack(idleStack);
    idleNode->pcbEntry.memoryFromMM = memoryForIdle;
    idleNode->pcbEntry.id=IDLE_ID;
    idleNode->pcbEntry.foreground = 1;
    idleNode->pcbEntry.state = READY;
    idleNode->pcbEntry.counterInit = 0;
    idleNode->pcbEntry.agingInterval = 0;
    idleNode->pcbEntry.quantums = 1;
    idle = idleNode;
    
    current = NULL;

    uint64_t CPUCristalClockSpeed = _getCPUCristalSpeed();
    uint64_t CPUTSCNumerator = _getTSCNumerator();
    uint64_t CPUTSCDenominator = _getTSCDenominator();

    // Check this 0x15 CPUID documentation https://sandpile.org/x86/cpuid.htm for more info about this calculation
    TSCFrequency = (CPUCristalClockSpeed*CPUTSCNumerator)/CPUTSCDenominator;
}

/*
    The next process will be:
        1. The first process that is on READY state
        2. The idle process that halts until any other process is on READY state
*/
static void nextProcess(){
    if(current->pcbEntry.id == idle->pcbEntry.id){
            current->pcbEntry.state = READY;
            current = NULL;
    }
    switch(current->pcbEntry.state){
        case BLOCKED: insertInBlockedQueue(current); break;
        case RUNNING: current->pcbEntry.state = READY; current->next = NULL; insertInQueue(current); break;
        case YIELDED: current->pcbEntry.state = READY; insertLast(current); break;
        default: break;
    }

    PCBNodeADT next;
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
        current->pcbEntry.agingInterval = 0;
        current->pcbEntry.quantums = 1;
        current->pcbEntry.state = RUNNING;
        return;
    }

    setForegroundProcess(&current->pcbEntry);

    current->next = NULL;
    current->pcbEntry.state = RUNNING;
}

static void insertInQueue(PCBNodeADT node){
    uint64_t agingTime = node->pcbEntry.agingInterval;
    int i;
    if(agingTime <= QUANTUM_SIZE){
        i = 0;
    } else if(agingTime <= QUANTUM_SIZE*2){
        i = 1;
    } else if(agingTime <= QUANTUM_SIZE*3){
        i = 2;
    } else if(agingTime <= QUANTUM_SIZE*4){
        i = 3;
    } else if(agingTime <= QUANTUM_SIZE*5){
        i = 4;
    } else {
        i = 5;
    }
    PCBNodeADT auxCurrent = multipleQueues[i]->head;
    PCBNodeADT previous = NULL;
    while(auxCurrent != NULL){
        previous = auxCurrent;
        auxCurrent = auxCurrent->next;
    }
    if(previous == NULL){
        multipleQueues[i]->head = node;
    } else{
        previous->next = node;
    }
    node->next = NULL;
    node->pcbEntry.quantums = multipleQueues[i]->defaultQuantums;
    node->pcbEntry.priority = i;
}

static void insertLast(PCBNodeADT process){
    PCBNodeADT auxCurrent = multipleQueues[5]->head;
    PCBNodeADT previous = NULL;
    while(auxCurrent != NULL){
        previous = auxCurrent;
        auxCurrent = auxCurrent->next;
    }
    if(previous == NULL){
        multipleQueues[5]->head = process;
    } else{
        previous->next = process;
    }
    process->next = NULL;
}

static void insertInBlockedQueue(PCBNodeCDT *node){
    PCBNodeADT auxCurrent = multipleQueues[6]->head;
    PCBNodeADT previous = NULL;

    node->next = NULL;
    while(auxCurrent != NULL){
        previous = auxCurrent;
        auxCurrent = auxCurrent->next;
    }

    if(previous == NULL){
        multipleQueues[6]->head = node;
    } else{
        previous->next = node;
    }
}

static int deleteProcess(PCBNodeADT toDelete){
    if(foreground == &toDelete->pcbEntry){
        foreground = NULL;
    }
    updateTimers(_readTimeStampCounter()/TSCFrequency);
    freeAllocations(&toDelete->pcbEntry);
    closePDs(&toDelete->pcbEntry);
    checkChilds(&toDelete->pcbEntry);
    setParentReady(&toDelete->pcbEntry);
    freeMemory(toDelete->pcbEntry.memoryFromMM);
    freePCB(toDelete);

    return toDelete->pcbEntry.id;
}

static void closePDs(ProcessControlBlockADT process){
    IPCBufferADT aux;
    for(int i=0; i<PD_SIZE ;i++){
        aux = process->pdTable[i];
        if(aux != NULL){
            process->pdTable[i] = NULL;
            removeReferences(aux, process->id);
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

    return max + 1 == 2 ? 3 : max + 1;
}

char exists(uint32_t pid) {
    PCBNodeCDT *aux;
    if(current->pcbEntry.id == pid || idle->pcbEntry.id == pid){
        return TRUE;
    }
    for(int i=0; i<7 ;i++){
        aux = multipleQueues[i]->head;
        while(aux != NULL){
            if(aux->pcbEntry.id == pid){
                return TRUE;
            }
            aux = aux->next;
        }
    }
    return FALSE;
}

int sysFork(void *currentProcessStack){
    PCBNodeADT newNode = allocPCB();
    if(newNode == NULL){
        return -1;
    }
    PCBNodeADT currentNode = current;
    void *memoryForFork = allocMemory(currentNode->pcbEntry.stackSize);
    if(memoryForFork == NULL){
        freePCB(newNode);
        return -1;
    }
    
    // We add 8 bytes to give space to the RAX return value
    uint64_t newStackOffset = (uint64_t)currentProcessStack - (uint64_t)currentNode->pcbEntry.memoryFromMM - 8;
    void *newStack = (void *)((uint64_t) memoryForFork + newStackOffset);
    newNode->pcbEntry.stack = newStack;

    uint64_t newBaseStackOffset = (uint64_t)currentNode->pcbEntry.baseStack - (uint64_t)currentNode->pcbEntry.memoryFromMM;
    newNode->pcbEntry.baseStack = (void *)((uint64_t)memoryForFork + newBaseStackOffset);

    newNode->pcbEntry.stackSize = currentNode->pcbEntry.stackSize;
    newNode->pcbEntry.memoryFromMM = memoryForFork;
    
    copyBlocks(newNode->pcbEntry.memoryFromMM, currentNode->pcbEntry.memoryFromMM);
    _setNewStack(newStack);

    newNode->pcbEntry.foreground = currentNode->pcbEntry.foreground;
    newNode->pcbEntry.state = currentNode->pcbEntry.state;
    newNode->pcbEntry.manualBlocked = currentNode->pcbEntry.manualBlocked;
    newNode->pcbEntry.priority = currentNode->pcbEntry.priority;

    newNode->pcbEntry.quantums = currentNode->pcbEntry.quantums;
    newNode->pcbEntry.agingInterval = currentNode->pcbEntry.agingInterval;
    newNode->pcbEntry.counterInit = currentNode->pcbEntry.counterInit;
    newNode->pcbEntry.firstAlloc = currentNode->pcbEntry.firstAlloc;
    newNode->pcbEntry.foreground = currentNode->pcbEntry.foreground;
    for(int i=0; i<PD_SIZE ;i++){
        IPCBufferADT iBuffer = currentNode->pcbEntry.pdTable[i];
        if(iBuffer != NULL){
            ProcessControlBlockADT aux;
            for(int j=0; j<PD_SIZE ;j++){
                aux = getReferenceByIndex(iBuffer, j);
                if(aux == NULL){
                    setReferenceByIndex(iBuffer, &newNode->pcbEntry, j);
                    break;
                }
            }
        }
        newNode->pcbEntry.pdTable[i] = currentNode->pcbEntry.pdTable[i];
    }

    // Should be other id
    uint64_t parentId = currentNode->pcbEntry.id;
    newNode->pcbEntry.id = unusedID();
    newNode->pcbEntry.parentId = parentId;
    int i=0;
    while(currentNode->pcbEntry.childsIds[i++] != 0);
    currentNode->pcbEntry.childsIds[i-1] = newNode->pcbEntry.id;

    insertInQueue(newNode);

    // We return the child process id to the parent, the child will return from an int20h
    return newNode->pcbEntry.id;
}

int sysExecve(processFunc process, int argc, char **argv, void *rsp){
    ProcessControlBlockCDT *currentProcess = &current->pcbEntry;

    // This routine will work only if its called from a syscall, because it manipulates the syscall stack frame
    if(_setProcess(process, argc, argv, rsp) == -1){
        return -1;
    }
    currentProcess->counterInit = 0;
    currentProcess->agingInterval = 0;
    currentProcess->priority = 0;
    currentProcess->quantums = 1;
    return 1;
}

int sysPs(ProcessData data[]) {
    int dim = 0;
    
    /* Current process */
    data[dim].id = current->pcbEntry.id;
    data[dim].stack = current->pcbEntry.stack;
    data[dim].baseStack = current->pcbEntry.baseStack;
    data[dim].priority = current->pcbEntry.priority;
    data[dim].foreground = current->pcbEntry.foreground;
    dim++;

    /* READY and BLOCKED processes */
    queue_t queues[7]  = {level0Queue, level1Queue, level2Queue, level3Queue, level4Queue, level5Queue, blockedList};
    PCBNodeCDT* current;
    for (int i = 0; i < 7; i++) {
        current = queues[i].head;
        while (current != NULL) {
            data[dim].id = current->pcbEntry.id;
            data[dim].stack = current->pcbEntry.stack;
            data[dim].baseStack = current->pcbEntry.baseStack;
            data[dim].priority = current->pcbEntry.priority;
            data[dim].foreground = current->pcbEntry.foreground;
            dim++;
            current = current->next;
        }
    }
    return dim;
}

ProcessControlBlockADT getEntry(uint32_t pid) {
    if (!exists(pid)) {
        return NULL;
    }

    if(current->pcbEntry.id == pid){
        return &current->pcbEntry;
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

int changePriority(uint32_t pid, unsigned int newPriority) {
    /* No se puede cambiar a un proceso a la lista blocked de esta forma. */
    if (!exists(pid) || newPriority >= 6) {
        return -1;
    }

    /* ¿Es el proceso current? En caso afirmativo, no está en ninguna lista, por ende,
        solo le cambiamos el priority y retornamos. 
    */
    PCBNodeCDT *toChange;

    if (current->pcbEntry.id == pid) {
        toChange = current;
    } else {
        toChange = removeFromQueue(pid);
    }

    switch(newPriority){
        case 0: toChange->pcbEntry.agingInterval = QUANTUM_SIZE*1; break;
        case 1: toChange->pcbEntry.agingInterval = QUANTUM_SIZE*2; break;
        case 2: toChange->pcbEntry.agingInterval = QUANTUM_SIZE*4; break;
        case 3: toChange->pcbEntry.agingInterval = QUANTUM_SIZE*8; break;
        case 4: toChange->pcbEntry.agingInterval = QUANTUM_SIZE*16; break;
        case 5: toChange->pcbEntry.agingInterval = QUANTUM_SIZE*32; break;
    }
    toChange->pcbEntry.priority = newPriority;
    toChange->pcbEntry.quantums = multipleQueues[newPriority]->defaultQuantums;
    
    if(toChange->pcbEntry.state == BLOCKED){
        insertInBlockedQueue(toChange);
    } else{
        insertInQueue(toChange);
    }
    return 1;

}

int hasOpenChilds(ProcessControlBlockADT entry){
    for(int i=0; i<PD_SIZE ;i++){
        if(entry->childsIds[i] != 0){
            ProcessControlBlockADT child = getEntry(entry->childsIds[i]);
            if(child->state != EXITED && child->foreground == TRUE){
                return TRUE;
            }
        }
    }
    return FALSE;
}

static PCBNodeADT removeFromQueue(uint32_t pid){
    PCBNodeADT toRemove;
    PCBNodeADT previous;
    for(int i=0; i<7 ;i++){
        toRemove = multipleQueues[i]->head;
        previous = NULL;
        while(toRemove != NULL){
            if(toRemove->pcbEntry.id == pid){
                if(previous == NULL){
                    multipleQueues[i]->head = toRemove->next;
                } else{
                    previous->next = toRemove->next;
                }
                toRemove->next = NULL;
                return toRemove;
            } else{
                previous = toRemove;
                toRemove = toRemove->next;
            }
        }
    }
    return NULL;
}

static void setParentReady(ProcessControlBlockADT pcbEntry){
    ProcessControlBlockADT parent = getEntry(pcbEntry->parentId);
    for(int i=0; i<PD_SIZE ;i++){
        if(parent->childsIds[i] == pcbEntry->id){
            parent->childsIds[i] = 0;
        }
    }
    
    setProcessState(parent, READY);
    return;
}

static void checkChilds(ProcessControlBlockADT pcbEntry){
    ProcessControlBlockADT parent = getEntry(pcbEntry->parentId);
    ProcessControlBlockADT child;
    for(int i=0; i<PD_SIZE ;i++){
        if(pcbEntry->childsIds[i] != 0){
            child = getEntry(pcbEntry->childsIds[i]);
            child->parentId = parent->id;
        }
    }
}

static void removeReferences(IPCBufferADT pdBuffer, uint32_t pid){
    ProcessControlBlockADT current;
    for(int i=0; i<PD_SIZE ;i++){
        current = getReferenceByIndex(pdBuffer, i);
        if(current != NULL && current->id == pid){
            if(setReferenceByIndex(pdBuffer, NULL, i) == TRUE && getBufferId(pdBuffer) == PIPE){
                closePipe(pdBuffer);
            }
        }
    }
}

PCBNodeADT getCurrentProcess(){
    return current;
}

ProcessControlBlockADT getCurrentProcessEntry(){
    return &current->pcbEntry;
}

uint32_t getCurrentProcessPid(){
    return current->pcbEntry.id;
}

IPCBufferADT getPDEntry(ProcessControlBlockADT entry, uint32_t pd){
    return entry->pdTable[pd];
}

int setProcessState(ProcessControlBlockADT process, ProcessState state){
    if(!exists(process->id) || (process->id <= IDLE_ID && state == EXITED)){
        return FALSE;
    }
    process->state = state;
    process->manualBlocked = FALSE;
    return TRUE;
}

uint64_t getPCBNodeSize(){
    return sizeof(PCBNodeCDT);
}


static void setForegroundProcess(ProcessControlBlockADT process){
    if(process->foreground == TRUE){
        foreground = process;
    }
}


void setProcessToForeground(ProcessControlBlockADT process){
    process->foreground = TRUE;
    if(foreground == NULL){
        foreground = process;
    }
}

void setProcessToBackground(ProcessControlBlockADT process){
    process->foreground = FALSE;
    if(foreground != NULL && foreground->id == process->id){
        foreground = NULL;
    }
}

ProcessControlBlockADT getForegroundProcess(){
    return foreground;
}

void removeFromPDs(ProcessControlBlockADT process, IPCBufferADT buffToRemove){
    for(int i=0; i < PD_SIZE ;i++){
        if(process->pdTable[i] == buffToRemove){
            process->pdTable[i] = NULL;
        }
    }
}

void sysBlock(ProcessControlBlockADT process) {
    if(!exists(process->id) || (process->id <= IDLE_ID)){
        return;
    }
    process->state = BLOCKED;
    process->manualBlocked = TRUE;
    return;
}

void sysUnBlock(ProcessControlBlockADT process) {
    if(!exists(process->id) || (process->id <= IDLE_ID)){
        return;
    }
    process->state = READY;
    process->manualBlocked = FALSE;
}


void *sysMalloc(ProcessControlBlockADT process, uint64_t size){
    void *allocation = allocMemory(size + sizeof(ProcessAllocations));
    if(allocation == NULL){
        return NULL;
    }
    void *toReturn = (void *)((uint64_t)allocation + sizeof(ProcessAllocations));
    ProcessAllocations *pAllocation = (ProcessAllocations *) allocation;
    pAllocation->allocation = toReturn;
    pAllocation->next = NULL;
    ProcessAllocations *current = process->firstAlloc;
    if(current != NULL){
        while(current->next != NULL){
            current = current->next;
        }
        current->next = pAllocation;
    } else{
        process->firstAlloc = pAllocation;
    }
    return toReturn;
}

void *sysRealloc(ProcessControlBlockADT process, void *toRealloc, uint64_t size){
    ProcessAllocations *auxCurrent = process->firstAlloc;
    ProcessAllocations *previous = NULL;
    while(auxCurrent != NULL && auxCurrent->allocation != toRealloc){
        previous = auxCurrent;
        auxCurrent = auxCurrent->next;
    }
    if(auxCurrent == NULL){    // Error
        return NULL;
    }
    ProcessAllocations *next = auxCurrent->next;
    auxCurrent = (ProcessAllocations *) reAllocMemory((void *) auxCurrent, size + sizeof(ProcessAllocations));
    if(auxCurrent == NULL){
        previous->next = next;
        return NULL;
    }
    auxCurrent->next = next;
    if(previous != NULL){
        previous->next = auxCurrent;
    }
    auxCurrent->allocation = (void *)((uint64_t)auxCurrent + sizeof(ProcessAllocations));
    return auxCurrent->allocation;
}
void sysFree(ProcessControlBlockADT process, void *toFree){
    ProcessAllocations *auxCurrent = process->firstAlloc;
    ProcessAllocations *previous = NULL;
    while(auxCurrent != NULL && auxCurrent->allocation != toFree){
        previous = auxCurrent;
        auxCurrent = auxCurrent->next;
    }
    if(auxCurrent == NULL){    // Error
        return;
    }
    if(previous != NULL){
        previous->next = auxCurrent->next;
    }
    freeMemory((void *) auxCurrent);
    return;
}

static void freeAllocations(ProcessControlBlockADT process){
    ProcessAllocations *auxCurrent = process->firstAlloc;
    if(auxCurrent == NULL){
        return;
    }
    ProcessAllocations *previous = NULL;
    while(auxCurrent != NULL){
        previous = auxCurrent;
        auxCurrent = auxCurrent->next;
        freeMemory(previous);
    }
}

uint32_t getProcessId(ProcessControlBlockADT process){
    return process->id;
}

uint64_t getTSCFrequency(){
    return TSCFrequency;
}

int dupPd(ProcessControlBlockADT process, uint64_t oldPd, uint64_t newPd){
    if(oldPd >= PD_SIZE || newPd >= PD_SIZE || process->pdTable[oldPd] != NULL){
        return -1;
    }
    process->pdTable[oldPd] = process->pdTable[newPd];
    process->pdTable[newPd] = NULL;
    return 0;
}

void sysClosePd(ProcessControlBlockADT process, IPCBufferADT toClose, uint32_t pd){
    if(pd < PD_SIZE && process != NULL){
        process->pdTable[pd] = NULL;
        if(getBufferId(toClose) == PIPE){
            removeReferences(toClose, process->id);
        }
    }
}

void setProcessPd(ProcessControlBlockADT process, IPCBufferADT buffer, uint64_t index){
    if(process == NULL || index >= PD_SIZE){
        return;
    }
    process->pdTable[index] = buffer;
}

int isInForeground(ProcessControlBlockADT process){
    return process->foreground;
}

int isBlocked(ProcessControlBlockADT process) {
    return process->state == BLOCKED;
}

int isReady(ProcessControlBlockADT process) {
    return process->state == READY;
}

int isManualBlocked(ProcessControlBlockADT process) {
    return process->manualBlocked == TRUE;
}

ProcessState getProcessState(ProcessControlBlockADT process){
    return process->state;
}