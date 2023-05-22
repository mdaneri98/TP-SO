#include <memoryManager.h>
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>

/* Structures */
typedef struct list {
    PCBNode *head;
    PCBNode *current;
} list_t;

/* Prototypes */
uint32_t unusedID();
char exists(uint32_t pid);
int add(ProcessControlBlockCDT newEntry);
ProcessControlBlockCDT *getEntry(uint32_t pid);


/* Global Variables */
list_t linkedList;
uint32_t entriesCount = 0;

int sysFork() {
    ProcessControlBlockCDT newEntry;
    void *newStack = allocMemory(4000);
    if(newStack == NULL){
        return -1;
    }
    newEntry.stackBase = (uint64_t *)newStack;
    copyState(&newStack, linkedList.current->pcbEntry.stack);
    newEntry.stack = (uint64_t *)newStack;
    newEntry.foreground = linkedList.current->pcbEntry.foreground;
    newEntry.priority = linkedList.current->pcbEntry.priority;
    newEntry.state = linkedList.current->pcbEntry.state;
    
    // Should be other id
    uint64_t parentId = linkedList.current->pcbEntry.id;
    newEntry.id = unusedID();
    
    // Add this process to the scheduler
    if(add(newEntry) == -1){
        freeMemory(newEntry.stackBase);
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
    void *stack = allocMemory(4000);
    PCBNode *node = allocPCB();

    node->previous = NULL;
    node->next = NULL;

    node->pcbEntry.stackBase = stack;
    node->pcbEntry.stack = createInitStack(stack);
    node->pcbEntry.id = 0;
    node->pcbEntry.priority = 0;
    node->pcbEntry.foreground = 1;
    node->pcbEntry.state = READY;
    linkedList.current = NULL;
    linkedList.head = node;
}

// ----------- Implementación Round-Robin sin prioridad ----------------
int add(ProcessControlBlockCDT newEntry) {
    PCBNode *previous = NULL;
    PCBNode *current = linkedList.head;
    while (current->next != NULL) {
            previous = current;
            current = current->next;
    }

    PCBNode *newNode = allocPCB();
    if(newNode == NULL){
        return -1;
    }

    newNode->pcbEntry.id = newEntry.id;
    newNode->pcbEntry.priority = newEntry.priority;
    newNode->pcbEntry.stack = newEntry.stack;
    newNode->pcbEntry.state = newEntry.state;
    newNode->previous = current;
    newNode->next = NULL;

    current->next = newNode;
    return 0;
}

/*
    The next process will be:
        1. The first process that is on READY state
        2. The head process if it's no process next to the current
*/
ProcessControlBlockCDT next() {
    PCBNode *current = linkedList.current;

    // If it's the last node, we move into the beginning of te list
    if (current->pcbEntry.state != READY && current->next == NULL) {
        current = linkedList.head;
    }

    while (current->next != NULL && current->pcbEntry.state != READY) {
        current = current->next;

        // If the last node isn't READY, we move also at the beginning
        if (current->next == NULL) {
            if (current->pcbEntry.state != READY) {
                current = linkedList.head;
            }
        }
    }
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
        freeMemory(current->pcbEntry.stackBase);
        freePCB(current);
    }

    return pid;
}


ProcessControlBlockCDT *getEntry(uint32_t pid) {
    if (&linkedList.head == NULL || !exists(pid)) {
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

uint64_t *scheduler(uint64_t *rsp) {
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
    }
    

    /* 
        If the process was BLOCKED or RUNNING, and scheduler function started, we do the same thing. 
        Get the next process with a READY state and run it.
    */
    processToRun = next();
    processToRun.state = RUNNING;
    return processToRun.stack;
}

