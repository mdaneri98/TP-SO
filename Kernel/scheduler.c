#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>
#include <keyboard.h>
#include <syscallDispatcher.h>
#include <libasm.h>
#include <video.h>

#define DEFAULT_PROCESS_STACK_SIZE 0x1900

/* Structures */
typedef struct level0Queue {
    PCBNode *head;
    PCBNode *last;
    PCBNode *current;
    PCBNode *waiter;
    uint8_t defaultQuantums;
} queue_t;

/* Prototypes */
uint32_t unusedID();
char exists(uint32_t pid);
int add(ProcessControlBlockCDT newEntry);
static void closePDs(pid);
static void removeReferences(buffer_t *pdBuffer, uint32_t pid);

/* Global Variables */
queue_t level0Queue = {NULL, NULL, NULL, NULL, 1};
queue_t level1Queue = {NULL, NULL, NULL, NULL, 2};
queue_t level2Queue = {NULL, NULL, NULL, NULL, 4};
queue_t level3Queue = {NULL, NULL, NULL, NULL, 8};
queue_t level4Queue = {NULL, NULL, NULL, NULL, 16};
queue_t level5Queue = {NULL, NULL, NULL, NULL, 32};
queue_t level6Queue = {NULL, NULL, NULL, NULL, 1};

PCBNode *current = NULL;

queue_t *multipleQueues[] = {&level0Queue, &level1Queue, &level2Queue, 
                                &level3Queue, &level4Queue, &level5Queue, &level6Queue};
uint32_t entriesCount = 0;

// CPU speed in MHz
uint32_t cpuSpeed;

int sysFork() {
    ProcessControlBlockCDT newEntry;
    void *memoryForFork = allocMemory(level0Queue.current->pcbEntry.stackSize);
    void *newStack = (void *)((uint64_t) memoryForFork + level0Queue.current->pcbEntry.stackSize);
    if(newStack == NULL){
        return -1;
    }
    newEntry.stackSize = level0Queue.current->pcbEntry.stackSize;
    newEntry.memoryFromMM = memoryForFork;
    newEntry.baseStack = newStack;
    copyState(&newStack, level0Queue.current->pcbEntry.stack);
    newEntry.stack = newStack;

    newEntry.foreground = level0Queue.current->pcbEntry.foreground;
    newEntry.priority = level0Queue.current->pcbEntry.priority;
    newEntry.state = level0Queue.current->pcbEntry.state;

    newEntry.quantums = level0Queue.current->pcbEntry.quantums;
    newEntry.agingInterval = level0Queue.current->pcbEntry.agingInterval;
    newEntry.currentInterval = level0Queue.current->pcbEntry.currentInterval;

    for(int i=0; i<PD_SIZE ;i++){
        newEntry.pdTable[i] = level0Queue.current->pcbEntry.pdTable[i];
        newEntry.readBuffer.references[i] = level0Queue.current->pcbEntry.readBuffer.references[i];
        newEntry.writeBuffer.references[i] = level0Queue.current->pcbEntry.writeBuffer.references[i];
    }
    
    for(int i=0; i<PD_BUFF_SIZE ;i++){
        newEntry.writeBuffer.buffer[i] = level0Queue.current->pcbEntry.writeBuffer.buffer[i];
        newEntry.readBuffer.buffer[i] = level0Queue.current->pcbEntry.readBuffer.buffer[i];
    }

    // Should be other id
    uint64_t parentId = level0Queue.current->pcbEntry.id;
    newEntry.id = unusedID();

    newEntry.writeBuffer.buffId = newEntry.id;
    newEntry.readBuffer.buffId = newEntry.id;
    

    // Add this process to the scheduler
    if(add(newEntry) == -1){
        freeMemory(newEntry.memoryFromMM);
        return -1;
    }

    // If we are in the parent process
    if(level0Queue.current->pcbEntry.id == parentId){
        return newEntry.id;
    }

    // We are in the child process
    return 0;
}

int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp){
    ProcessControlBlockCDT *currentProcess = &level0Queue.current->pcbEntry;

    // IT ONLY SETUPS THE PROCESS CORRECTLY IF ITS CALLED BY THE SYSCALL, U NEED TO SET THE STACK
    //BEFORE USING IT WITH INIT OR ANY FUNCTIONALITY THAT USES THIS FUNCTION WITHOUT USING THE SYSCALL
    if(setProcess(process, argc, argv, rsp) == -1){
        return -1;
    }
    currentProcess->stack = rsp;
    currentProcess->currentInterval = 0;
    currentProcess->agingInterval = 0;
    currentProcess->priority = 0;
    for(int i=0; i<PD_BUFF_SIZE ;i++){
        currentProcess->readBuffer.buffer[i] = '\0';
        currentProcess->writeBuffer.buffer[i] = '\0';
    }
    return 1;
}

// FIXME: Implementar.
int sysKill(uint32_t pid) {
    if (pid == 0) {
        return -1;
    }

    //...
    return 0;
}

// FIXME: Implementar.
int sysBlock(uint32_t pid) {
    ProcessControlBlockCDT *PCBEntry = getEntry(pid);
    if(PCBEntry == NULL){
        return -1;
    }
    PCBEntry->state = BLOCKED;

    // We call manually the scheduler via the timer tick interrupt (to enshure that the stack-frame is done correctly)
    int20h();
    return 0;
}

void createInit() {
    void *memoryForInit = allocMemory(DEFAULT_PROCESS_STACK_SIZE);
    void *initStack = (void *)((uint64_t) memoryForInit + DEFAULT_PROCESS_STACK_SIZE);
    PCBNode *initNode = allocPCB();
    void *memoryForWaiter = allocMemory(DEFAULT_PROCESS_STACK_SIZE);
    void *waiterStack = (void *)((uint64_t) memoryForWaiter + DEFAULT_PROCESS_STACK_SIZE);
    PCBNode *waiterNode = allocPCB();
    
    initNode->previous = NULL;
    initNode->next = NULL;
    waiterNode->previous = NULL;
    waiterNode->next = NULL;

    initNode->pcbEntry.pdTable[0] = getSTDIN();
    initNode->pcbEntry.pdTable[0]->references[0] = &initNode->pcbEntry;

    initNode->pcbEntry.pdTable[1] = getSTDOUT();
    initNode->pcbEntry.pdTable[1]->references[0] = &initNode->pcbEntry;

    initNode->pcbEntry.pdTable[2] = getSTDERR();
    initNode->pcbEntry.pdTable[2]->references[0] = &initNode->pcbEntry;
    
    for(int i=3; i<PD_SIZE ;i++){
        initNode->pcbEntry.pdTable[i] = NULL;
        waiterNode->pcbEntry.pdTable[i] = NULL;
    }
    initNode->pcbEntry.stackSize = DEFAULT_PROCESS_STACK_SIZE;
    initNode->pcbEntry.baseStack = initStack;
    initNode->pcbEntry.stack = createInitStack(initStack);
    initNode->pcbEntry.memoryFromMM = memoryForInit;
    initNode->pcbEntry.id = 1;
    initNode->pcbEntry.priority = 0;
    initNode->pcbEntry.foreground = 1;
    initNode->pcbEntry.state = READY;
    initNode->pcbEntry.currentInterval = 0;
    initNode->pcbEntry.agingInterval = 0;
    initNode->pcbEntry.quantums = 1;
    level0Queue.current = NULL;
    level0Queue.head = initNode;
    level0Queue.last = initNode;

    waiterNode->pcbEntry.stackSize = DEFAULT_PROCESS_STACK_SIZE;
    waiterNode->pcbEntry.baseStack = waiterStack;
    waiterNode->pcbEntry.stack = createWaiterStack(waiterStack);
    waiterNode->pcbEntry.memoryFromMM = memoryForWaiter;
    waiterNode->pcbEntry.id=2;
    waiterNode->pcbEntry.priority = 0;
    waiterNode->pcbEntry.foreground = 1;
    waiterNode->pcbEntry.state = READY;
    waiterNode->pcbEntry.currentInterval = 0;
    waiterNode->pcbEntry.agingInterval = 0;
    waiterNode->pcbEntry.quantums = 1;
    level0Queue.waiter = waiterNode;
    
    level1Queue.current = NULL;
    level1Queue.head = NULL;
    level1Queue.last = NULL;
    level1Queue.waiter = NULL;

    level2Queue.current = NULL;
    level2Queue.head = NULL;
    level2Queue.last = NULL;
    level2Queue.waiter = NULL;

    level3Queue.current = NULL;
    level3Queue.head = NULL;
    level3Queue.last = NULL;
    level3Queue.waiter = NULL;

    level4Queue.current = NULL;
    level4Queue.head = NULL;
    level4Queue.last = NULL;
    level4Queue.waiter = NULL;

    level5Queue.current = NULL;
    level5Queue.head = NULL;
    level5Queue.last = NULL;
    level5Queue.waiter = NULL;

    level6Queue.head = waiterNode;
    level1Queue.current = NULL;
    level1Queue.last = NULL;
    level1Queue.waiter = NULL;
    
    cpuSpeed = (uint32_t) getCPUSpeed();
    // We pass the CPU speed from MHz to KHz (it makes the calculus of intervals in ms easier)
    cpuSpeed *= 1000;
}

// ----------- Implementación Round-Robin sin prioridad ----------------
int add(ProcessControlBlockCDT newEntry) {
    PCBNode *last = level0Queue.last;

    PCBNode *newNode = allocPCB();
    if(newNode == NULL){
        return -1;
    }

    for(int i=0; i<PD_SIZE ;i++){
        newNode->pcbEntry.pdTable[i] = newEntry.pdTable[i];
        if(newNode->pcbEntry.pdTable[i] != NULL){
            for(int j=0; j<PD_SIZE ;j++){
                if(newNode->pcbEntry.pdTable[i]->references[j] == NULL){
                    newNode->pcbEntry.pdTable[i]->references[j] = &newNode->pcbEntry;
                    break;
                }
            }
        }
    }
    newNode->pcbEntry.stackSize = newEntry.stackSize;
    newNode->pcbEntry.memoryFromMM = newEntry.memoryFromMM;
    newNode->pcbEntry.id = newEntry.id;
    newNode->pcbEntry.priority = newEntry.priority;
    newNode->pcbEntry.stack = newEntry.stack;
    newNode->pcbEntry.baseStack = newEntry.baseStack;
    newNode->pcbEntry.state = newEntry.state;
    newNode->pcbEntry.currentInterval = newEntry.currentInterval;
    newNode->pcbEntry.agingInterval = newEntry.agingInterval;
    newNode->previous = last;

    for(int i=0; i<PD_BUFF_SIZE ;i++){
        newNode->pcbEntry.writeBuffer.buffer[i] = newEntry.writeBuffer.buffer[i];
        newNode->pcbEntry.readBuffer.buffer[i] = newEntry.readBuffer.buffer[i];
    }

    last->next = newNode;
    newNode->next = NULL;

    level0Queue.last = newNode;
    return 0;
}

/*
    The next process will be:
        1. The first process that is on READY state
        2. The head process if it's no process next to the current
*/
ProcessControlBlockCDT next() {
    if(current->pcbEntry.id == level0Queue.waiter->pcbEntry.id){
            current->pcbEntry.state = READY;
    } else if(current->pcbEntry.state == RUNNING){
        current->previous->next = current->next;
        current->next->previous = current->previous;
        PCBNode *last = level0Queue.last;
        last->next = current;
        current->previous = last;
        current->pcbEntry.state = READY;
        current->next = NULL;
        level0Queue.last = current;
    }
    current = level0Queue.head;
    while (current != NULL && current->pcbEntry.state != READY) {
        current = current->next;
    }
    // If the last node isn't READY, we set the waiter
    if (current == NULL) {
        current = level0Queue.waiter;
    }

    current->pcbEntry.state = RUNNING;

    return current->pcbEntry;
}


int remove(uint32_t pid) {
    if (!exists(pid)) {
        return -1;
    }

    PCBNode *toRemove = current;
    if (toRemove->pcbEntry.id != pid && toRemove->next == NULL) {
        toRemove = level0Queue.head;
    }

    while (toRemove->pcbEntry.id != pid && toRemove->next != NULL) {
        toRemove = toRemove->next;

        if (toRemove->next == NULL) {
            if (toRemove->pcbEntry.id != pid) {
                toRemove = level0Queue.head;
            }
        }
    }

    if (toRemove->pcbEntry.id == pid) {
        toRemove->previous->next = toRemove->next;
        toRemove->next->previous = toRemove->previous;
        closePDs(pid);
        freeMemory(toRemove->pcbEntry.memoryFromMM);
        freePCB(toRemove);
    }

    return pid;
}


ProcessControlBlockCDT *getEntry(uint32_t pid) {
    if (level0Queue.head == NULL || !exists(pid)) {
        return NULL;
    }

    PCBNode *current = level0Queue.head;
    if (current->pcbEntry.id == pid)
        return &current->pcbEntry; 

    while (current->next != NULL) {
        current = current->next;
        if (current->pcbEntry.id == pid)
            return &current->pcbEntry; 
    }

    return NULL;
}

// It always has to be at least one process on the list
uint32_t unusedID() {
    if (level0Queue.head == NULL) {
        return -1;
    }

    PCBNode *node = level0Queue.head;
    uint32_t max = node->pcbEntry.id;

    while (node->next != NULL) {
        node = current->next;

        if (max < node->pcbEntry.id) {
            max = node->pcbEntry.id;
        }
    }

    return max + 1;
}

char exists(uint32_t pid) {
    if (level0Queue.head == NULL) {
        return -1;
    }

    char found = 0;
    PCBNode *node = level0Queue.head;
    found = node->pcbEntry.id == pid ? 1 : 0; 

    while (node->next != NULL && !found) {
        node = node->next;
        found = node->pcbEntry.id == pid ? 1 : 0; 
    }

    return found;
}

void *scheduler(void *rsp) {
    ProcessControlBlockCDT processToRun;
    // When the system starts up, the current field is always NULL
    if(current == NULL){
        current = level0Queue.head;
        processToRun = level0Queue.head->pcbEntry;
        processToRun.state = RUNNING;
        return processToRun.stack;
    }

    // Backup of the caller stack
    current->pcbEntry.stack = rsp;
    uint64_t cicles = readTimeStampCounter();
    current->pcbEntry.agingInterval /= 2;
    current->pcbEntry.agingInterval += (cicles - level0Queue.current->pcbEntry.currentInterval)/cpuSpeed;

    // If this function was called by a process, we need to check its state
    if (current->pcbEntry.state == EXITED) {
        PCBNode *aux = current;
        
        // We move to the next process that needs execution
        processToRun = next();

        current->pcbEntry.currentInterval = readTimeStampCounter()/cpuSpeed;
        
        // We remove the EXITED process from the list of processes
        remove(aux->pcbEntry.id);

        return processToRun.stack;
    }

    /* 
        If the process was BLOCKED or RUNNING, and scheduler function started, we do the same thing. 
        Get the next process with a READY state and run it.
    */
    processToRun = next();
    level0Queue.current->pcbEntry.currentInterval = readTimeStampCounter()/cpuSpeed;

    return processToRun.stack;
}

static void closePDs(uint32_t pid){
    if(pid == 0){
        // pid 0 belongs to standart input/output, can't close
        return;
    }
    PCBNode *node = level0Queue.head;
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

static void removeReferences(buffer_t *pdBuffer, uint32_t pid){
    for(int i=0; i<PD_SIZE ;i++){
        if(pdBuffer->references[i] != NULL && pdBuffer->references[i]->id == pid){
            pdBuffer->references[i] == NULL;
        }
    }
}

PCBNode *getCurrentProcess(){
    return current;
}