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

#define DEFAULT_PROCESS_STACK_SIZE 0x5000
#define QUANTUM_SIZE 55

#define INIT_ID 1
#define IDLE_ID 2

#define TRUE 1
#define FALSE 0

typedef struct PCBNodeCDT *PCBNodeADT;

typedef struct ProcessControlBlockCDT {
    uint32_t id;
    uint8_t foreground;
    ProcessState state;
    uint8_t priority;

    // Variables neccesary for computing priority scheduling
    uint8_t quantums;
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

    IPCBufferADT pdTable[PD_SIZE];
} ProcessControlBlockCDT;

/* Structures */
typedef struct PCBNodeCDT {
    PCBNodeADT next;
    PCBNodeADT previous;
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
static void setForegroundProcess(ProcessControlBlockADT process);

/* Global Variables */
queue_t level0Queue = {NULL, 1};
queue_t level1Queue = {NULL, 2};
queue_t level2Queue = {NULL, 4};
queue_t level3Queue = {NULL, 8};
queue_t level4Queue = {NULL, 16};
queue_t level5Queue = {NULL, 32};
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

    updateTimers();
    checkBlocked();
    checkExited();

    /* 
        If the process was BLOCKED or RUNNING, and scheduler function started, we do the same thing. 
        Get the nextProcess process with a READY state and run it.
    */
    current->pcbEntry.quantums--;
    if(current->pcbEntry.quantums < 1 || current->pcbEntry.state == BLOCKED){
        nextProcess();
    }
    current->pcbEntry.counterInit = _readTimeStampCounter();

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
                deleteProcess(toRemove);
            }
            toRemove = toRemove->next;
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
    PCBNodeCDT *initNode = allocPCB();
    void *memoryForIdle = allocMemory(DEFAULT_PROCESS_STACK_SIZE);
    void *idleStack = (void *)((uint64_t) memoryForIdle + DEFAULT_PROCESS_STACK_SIZE);
    PCBNodeCDT *idleNode = allocPCB();
    
    initNode->previous = NULL;
    initNode->next = NULL;
    idleNode->previous = NULL;
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
    initNode->pcbEntry.stackSize = DEFAULT_PROCESS_STACK_SIZE;
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
    level0Queue.head = initNode;

    foreground = initNode;

    idleNode->pcbEntry.stackSize = DEFAULT_PROCESS_STACK_SIZE;
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
    } else if(current->pcbEntry.state == BLOCKED){
        insertInBlockedQueue(current);
    } else if(current->pcbEntry.state == RUNNING){
        current->pcbEntry.state = READY;
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
        current->pcbEntry.agingInterval = 0;
        current->pcbEntry.quantums = 1;
        current->pcbEntry.state = RUNNING;
        return;
    }

    if(current->pcbEntry.foreground == TRUE){
        foreground = &current->pcbEntry;
    }

    current->next = NULL;
    current->previous = NULL;
    current->pcbEntry.state = RUNNING;
}

static void insertInQueue(PCBNodeCDT *node){
    uint64_t agingTime = node->pcbEntry.agingInterval;
    int i;
    if(agingTime <= QUANTUM_SIZE){
        i = 0;
    } else if(agingTime <= QUANTUM_SIZE*2){
        i = 1;
    } else if(agingTime <= QUANTUM_SIZE*4){
        i = 2;
    } else if(agingTime <= QUANTUM_SIZE*8){
        i = 3;
    } else if(agingTime <= QUANTUM_SIZE*16){
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
    node->previous = NULL;
    node->next = NULL;
    while(first != NULL && first->next != NULL){
        first = first->next;
    }
    if(first == NULL){
        multipleQueues[6]->head = node;
    } else{
        first->next = node;
        node->previous = first;
    }
}

static int deleteProcess(PCBNodeCDT *toDelete){
    closePDs(toDelete->pcbEntry.id);
    checkChilds(&toDelete->pcbEntry);
    setParentReady(&toDelete->pcbEntry);
    freeMemory(toDelete->pcbEntry.memoryFromMM);
    freePCB(toDelete);

    return toDelete->pcbEntry.id;
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
        IPCBufferADT aux;
        for(int i=0; i<PD_SIZE ;i++){
            aux = node->pcbEntry.pdTable[i];
            if(aux != NULL){
                node->pcbEntry.pdTable[i] = NULL;
                removeReferences(node, pid);
            }
        }
        node = node->next;
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
    PCBNodeCDT *newNode = allocPCB();
    if(newNode == NULL){
        return -1;
    }
    PCBNodeCDT *currentNode = current;
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
    newNode->pcbEntry.priority = currentNode->pcbEntry.priority;

    newNode->pcbEntry.quantums = currentNode->pcbEntry.quantums;
    newNode->pcbEntry.agingInterval = currentNode->pcbEntry.agingInterval;
    newNode->pcbEntry.counterInit = currentNode->pcbEntry.counterInit;

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
    currentNode->pcbEntry.childsIds[i] = newNode->pcbEntry.id;

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
            ProcessControlBlockADT child = entry->childsIds[i];
            if(child->state != EXITED){
                return TRUE;
            }
        }
    }
    return FALSE;
}

static PCBNodeCDT *removeFromQueue(uint32_t pid){
    PCBNodeCDT *toRemove;
    for(int i=0; i<7 ;i++){
        toRemove = multipleQueues[i]->head;
        while(toRemove != NULL){
            if(toRemove->pcbEntry.id == pid){
                if(toRemove->pcbEntry.id == multipleQueues[i]->head->pcbEntry.id){
                    multipleQueues[i]->head = toRemove->next;
                    toRemove->next->previous = NULL;
                    toRemove->next = NULL;
                } else{
                    toRemove->previous->next = toRemove->next;
                    if(toRemove->next != NULL){
                        toRemove->next->previous = toRemove->previous;
                    }
                }
                return toRemove;
            } else{
                toRemove = toRemove->next;
            }
        }
    }
}

static void setParentReady(ProcessControlBlockADT pcbEntry){
    ProcessControlBlockADT parent = getEntry(pcbEntry->parentId);
    for(int i=0; i<PD_SIZE ;i++){
        parent->childsIds[i] = parent->childsIds[i] == pcbEntry->id ? 0 : parent->childsIds[i];
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
            setReferenceByIndex(pdBuffer, NULL, i);
        }
    }
}

/* Alterna entre los estados READY y BLOCKED para el proceso dado. */
int changeState(uint32_t pid) {
    if (!exists(pid)){
        return -1;
    }

    PCBNodeCDT* entry = getEntry(pid);
    int priority = entry->pcbEntry.priority;
    deleteProcess(entry);  // Lo eliminamos de la lista actual, para luego, agregarlo a la lista requerida.
    
    if (entry->pcbEntry.state == BLOCKED) {
        PCBNodeCDT* current = multipleQueues[priority]->head;
        if (current == NULL) {
            multipleQueues[priority]->head = entry;
        } else {   
            while (current->next != NULL) {
                current = current->next;
            }
            entry->pcbEntry.state = READY;
            current->next = entry;
        }    
    } else {
        PCBNodeCDT* current = multipleQueues[6]->head;
        if (current == NULL) {
            multipleQueues[6]->head = entry;
        } else {   
            while (current->next != NULL) {
                current = current->next;
            }
            entry->pcbEntry.state = BLOCKED;
            current->next = entry;
        }    
    }
    return 1;
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
}

void setProcessToBackground(ProcessControlBlockADT process){
    process->foreground = FALSE;
    if(foreground->id == process->id){
        foreground == NULL;
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

uint32_t getProcessId(ProcessControlBlockADT process){
    return process->id;
}

uint64_t getTSCFrequency(){
    return TSCFrequency;
}