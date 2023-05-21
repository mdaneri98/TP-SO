#include <freeListMemoryManager.h>  //FIXME: Debería ser uno general.
#include <process.h>
#include <scheduler.h>

/* Structures */
typedef struct list {
    PCBNode *head;
    PCBNode *current;
} list_t;

/* Prototypes */
int32_t unusedID();
char exists(uint32_t pid);
void add(ProcessControlBlockCDT newEntry);


/* Global Variables */
list_t linkedList;
uint32_t entriesCount = 0;

int sysFork() {
    ProcessControlBlockCDT entry;
    void *newStack = allocMemory(4000);
    if(newStack == NULL){
        return -1;
    }
    copyState(&newStack, linkedList.current->pcbEntry.stack);
    entry.stack = newStack;
    entry.foreground = linkedList.current->pcbEntry.foreground;
    entry.priority = linkedList.current->pcbEntry.priority;
    entry.state = linkedList.current->pcbEntry.state;
    
    // Should be other id
    uint64_t parentId = linkedList.current->pcbEntry.id;
    entry.id = unusedID();
    
    // Add this process to the scheduler
    add(entry);

    // If we are in the parent process
    if(linkedList.current->pcbEntry.id == parentId){
        return entry.id;
    }
    return 0;
}

int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp){
    // ProcessControlBlockCDT currentProcess = linkedList.current.pcbEntry;
    

    // IT ONLY SETUPS THE PROCESS CORRECTLY IF ITS CALLED BY THE SYSCALL, U NEED TO SET THE STACK
    //BEFORE USING IT WITH INIT OR ANY FUNCTIONALITY THAT USES THIS FUNCTION WITHOUT USING THE SYSCALL
    return setProcess(process, argc, argv, rsp);
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
    //...
    return 0;
}

void createInit() {
    void *stack = allocMemory(4000);
    PCBNode *node = allocPCB();

    node->previous = NULL;
    node->next = NULL;

    node->pcbEntry.stack = createInitStack(stack);
    node->pcbEntry.id = 0;
    node->pcbEntry.priority = 0;
    node->pcbEntry.foreground = 1;
    node->pcbEntry.state = READY;
    linkedList.current = NULL;
    linkedList.head = node;
}

// ----------- Implementación Round-Robin sin prioridad ----------------
void add(ProcessControlBlockCDT newEntry) {
    PCBNode *previous = NULL;
    PCBNode *current = linkedList.head;
    while (current->next != NULL) {
            previous = current;
            current = current->next;
    }

    PCBNode *newNode = allocPCB();

    newNode->pcbEntry.id = newEntry.id;
    newNode->pcbEntry.priority = newEntry.priority;
    newNode->pcbEntry.stack = newEntry.stack;
    newNode->pcbEntry.state = newEntry.state;
    newNode->previous = current;
    newNode->next = NULL;

    current->next = newNode;
}

/*
El siguiente proceso a ejecutar será:
    1. El primer next que esté en estado READY.
    2. El head si no hay next al current.
*/
ProcessControlBlockCDT next() {
    PCBNode *current = linkedList.current;

    // Si estamos en el último nodo, nos movemos al inicio.
    if (current->pcbEntry.state != READY && current->next == NULL) {
        current = linkedList.head;
    }

    while (current->next != NULL && current->pcbEntry.state != READY) {
        current = current->next;

        /* Si es el último nodo, y no está READY, volvemos al inicio. */
        if (current->next == NULL) {
            if (current->pcbEntry.state != READY) {
                current = &linkedList.head;
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
        freePCB(current);
    }

    return pid;
}


ProcessControlBlockCDT* getEntry(uint32_t pid) {
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

/* Siempre debe haber al menos un proceso en la lista. */
int32_t unusedID() {
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
    PCBNode *current = &linkedList.head;
    founded = current->pcbEntry.id == pid ? 1 : 0; 

    while (current->next != NULL && !founded) {
        current = current->next;
        founded = current->pcbEntry.id == pid ? 1 : 0; 
    }

    return founded;
}

void scheduler(uint64_t *rsp) {
    ProcessControlBlockCDT processToRun;
    if(linkedList.current == NULL){
        linkedList.current = linkedList.head;
        processToRun = linkedList.head->pcbEntry;
        processToRun.state = RUNNING;
        return processToRun.stack;
    }
    
    linkedList.current->pcbEntry.stack = rsp;
    /* Si fue llamado manualmente, mediante int 0x20, tenemos que
        revisar el estado de este proceso */
    if (linkedList.current->pcbEntry.state == EXITED) {
        PCBNode *aux = linkedList.current;
        
        //Movemos el puntero al siguiente en ejecutar.
        processToRun = next(); 
        
        //Eliminamos el proceso actual (que está en estado EXITED) de la lista.
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

