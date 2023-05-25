#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>
#include <keyboard.h>
#include <syscallDispatcher.h>
#include <libasm.h>
#include <video.h>

/* Structures */
typedef struct list {
    PCBNode *head;
    PCBNode *last;
    PCBNode *current;
    PCBNode *waiter;
} list_t;

/* Prototypes */
uint32_t unusedID();
char exists(uint32_t pid);
int add(ProcessControlBlockCDT newEntry);
static void closePDs(pid);
static void removeReferences(buffer_t *pdBuffer, uint32_t pid);

/* Global Variables */
list_t linkedList;
uint32_t entriesCount = 0;

// CPU speed in MHz
uint32_t cpuSpeed;

int sysFork() {
    ProcessControlBlockCDT newEntry;
    void *memoryForFork = allocMemory(0x1900);
    void *newStack = (void *)((uint64_t) memoryForFork + 0x1900);
    if(newStack == NULL){
        return -1;
    }
    newEntry.memoryFromMM = memoryForFork;
    newEntry.baseStack = newStack;
    copyState(&newStack, linkedList.current->pcbEntry.stack);
    newEntry.stack = newStack;
    newEntry.foreground = linkedList.current->pcbEntry.foreground;
    newEntry.priority = linkedList.current->pcbEntry.priority;
    newEntry.state = linkedList.current->pcbEntry.state;
    for(int i=0; i<PD_SIZE ;i++){
        newEntry.pdTable[i] = linkedList.current->pcbEntry.pdTable[i];
    }
    
    // Should be other id
    uint64_t parentId = linkedList.current->pcbEntry.id;
    newEntry.id = unusedID();
    
    // Add this process to the scheduler
    if(add(newEntry) == -1){
        freeMemory(newEntry.baseStack);
        return -1;
    }

    // If we are in the parent process
    if(linkedList.current->pcbEntry.id == parentId){
        return newEntry.id;
    }

    // We are in the child process
    return 0;
}

int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp){
    ProcessControlBlockCDT currentProcess = linkedList.current->pcbEntry;

    // IT ONLY SETUPS THE PROCESS CORRECTLY IF ITS CALLED BY THE SYSCALL, U NEED TO SET THE STACK
    //BEFORE USING IT WITH INIT OR ANY FUNCTIONALITY THAT USES THIS FUNCTION WITHOUT USING THE SYSCALL
    if(setProcess(process, argc, argv, rsp) == -1){
        return -1;
    }
    currentProcess.stack = (uint64_t *)rsp;
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
    void *memoryForInit = allocMemory(0x1900);
    void *initStack = (void *)((uint64_t) memoryForInit + 0x1900);
    PCBNode *initNode = allocPCB();
    void *memoryForWaiter = allocMemory(0x1900);
    void *waiterStack = (void *)((uint64_t) memoryForWaiter + 0x1900);
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
    initNode->pcbEntry.baseStack = initStack;
    initNode->pcbEntry.stack = createInitStack(initStack);
    initNode->pcbEntry.id = 1;
    initNode->pcbEntry.priority = 0;
    initNode->pcbEntry.foreground = 1;
    initNode->pcbEntry.state = READY;
    linkedList.current = NULL;
    linkedList.head = initNode;
    linkedList.last = initNode;

    waiterNode->pcbEntry.baseStack = waiterStack;
    waiterNode->pcbEntry.stack = createWaiterStack(waiterStack);
    waiterNode->pcbEntry.id=2;
    waiterNode->pcbEntry.priority = 0;
    waiterNode->pcbEntry.foreground = 1;
    waiterNode->pcbEntry.state = READY;
    linkedList.waiter = waiterNode;
    
    cpuSpeed = (uint32_t) getCPUSpeed();
    // We pass the CPU speed from MHz to KHz (it makes the calculus of intervals in ms easier)
    cpuSpeed *= 1000;
}

// ----------- Implementación Round-Robin sin prioridad ----------------
int add(ProcessControlBlockCDT newEntry) {
    PCBNode *last = linkedList.last;

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
    newNode->pcbEntry.memoryFromMM = newEntry.memoryFromMM;
    newNode->pcbEntry.id = newEntry.id;
    newNode->pcbEntry.priority = newEntry.priority;
    newNode->pcbEntry.stack = newEntry.stack;
    newNode->pcbEntry.baseStack = newEntry.baseStack;
    newNode->pcbEntry.state = newEntry.state;
    newNode->previous = last;
    last->next = newNode;
    newNode->next = NULL;

    linkedList.last = newNode;
    return 0;
}

/*
    The next process will be:
        1. The first process that is on READY state
        2. The head process if it's no process next to the current
*/
ProcessControlBlockCDT next() {
    PCBNode *current = linkedList.current;
    if(current->pcbEntry.id == linkedList.waiter->pcbEntry.id){
            current->pcbEntry.state = READY;
    } else if(current->pcbEntry.state == RUNNING){
        current->previous->next = current->next;
        current->next->previous = current->previous;
        PCBNode *last = linkedList.last;
        last->next = current;
        current->previous = last;
        current->pcbEntry.state = READY;
        current->next = NULL;
        linkedList.last = current;
    }
    current = linkedList.head;
    while (current != NULL && current->pcbEntry.state != READY) {
        current = current->next;
    }
    // If the last node isn't READY, we set the waiter
    if (current == NULL) {
        current = linkedList.waiter;
    }

    current->pcbEntry.state = RUNNING;
    linkedList.current = current;

    return linkedList.current->pcbEntry;
}


int remove(uint32_t pid) {
    if (!exists(pid)) {
        return -1;
    }

    PCBNode *current = linkedList.current;
    if (current->pcbEntry.id != pid && current->next == NULL) {
        current = linkedList.head;
    }

    while (current->pcbEntry.id != pid && current->next != NULL) {
        current = current->next;

        if (current->next == NULL) {
            if (current->pcbEntry.id != pid) {
                current = linkedList.head;
            }
        }
    }

    if (current->pcbEntry.id == pid) {
        current->previous->next = current->next;
        current->next->previous = current->previous;
        closePDs(pid);
        freeMemory(current->pcbEntry.memoryFromMM);
        freePCB(current);
    }

    return pid;
}


ProcessControlBlockCDT *getEntry(uint32_t pid) {
    if (linkedList.head == NULL || !exists(pid)) {
        return NULL;
    }

    PCBNode *current = linkedList.head;
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
    if (linkedList.head == NULL) {
        return -1;
    }

    PCBNode *current = linkedList.head;
    uint32_t max = current->pcbEntry.id;

    while (current->next != NULL) {
        current = current->next;

        if (max < current->pcbEntry.id) {
            max = current->pcbEntry.id;
        }
    }

    return max + 1;
}

char exists(uint32_t pid) {
    if (linkedList.head == NULL) {
        return -1;
    }

    char founded = 0;
    PCBNode *current = linkedList.head;
    founded = current->pcbEntry.id == pid ? 1 : 0; 

    while (current->next != NULL && !founded) {
        current = current->next;
        founded = current->pcbEntry.id == pid ? 1 : 0; 
    }

    return founded;
}

void *scheduler(void *rsp) {
    ProcessControlBlockCDT processToRun;
    // When the system starts up, the current field is always NULL
    if(linkedList.current == NULL){
        linkedList.current = linkedList.head;
        processToRun = linkedList.head->pcbEntry;
        processToRun.state = RUNNING;
        return processToRun.stack;
    }

    // Backup of the caller stack
    linkedList.current->pcbEntry.stack = rsp;

    // If this function was called by a process, we need to check its state
    if (linkedList.current->pcbEntry.state == EXITED) {
        PCBNode *aux = linkedList.current;
        
        // We move to the next process that needs execution
        processToRun = next();
        
        // We remove the EXITED process from the list of processes
        remove(aux->pcbEntry.id);

        return processToRun.stack;
    }

    /* 
        If the process was BLOCKED or RUNNING, and scheduler function started, we do the same thing. 
        Get the next process with a READY state and run it.
    */
    processToRun = next();

    return processToRun.stack;
}

static void closePDs(uint32_t pid){
    if(pid == 0){
        // pid 0 belongs to standart input/output, can't close
        return;
    }
    PCBNode *node = linkedList.head;
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
    return linkedList.current;
}