#include <stdlib.h>
#include <freeListMemoryManager.h>  //FIXME: Debería ser uno general.
#include "process.h"
#include "scheduler.h"

/* Prototypes */
int32_t unusedID();
char exists(uint32_t pid);


/* Structures */
typedef struct ProcessControlBlockCDT {
    char* name;
    unsigned int id;
    unsigned int priority;
    char foreground;
    ProcessState state;
    uint64_t *stack;
} ProcessControlBlockCDT;

typedef struct node {
    struct node *next;
    struct node *previous;
    ProcessControlBlockCDT pcbEntry;
} node_t;

typedef struct list {
    node_t head;
    node_t current;
} list_t;

list_t linkedList;

uint32_t entriesCount = 0;
char firstTime = 1;

static uint64_t currentId = 0;

ProcessControlBlockCDT createInit() {
    node_t *node = PCB_TABLE;

    ProcessControlBlockCDT *pcbEntry = &(node->pcbEntry);
    pcbEntry->name = "init"; //String en zona de datos.
    pcbEntry->id = 0;
    pcbEntry->priority = 0;
    pcbEntry->foreground = 1;
    pcbEntry->state = RUNNING;
}

int sysFork(){
    ProcessControlBlockCDT entry;
    void *newStack = allocMemory(4096);
    if(newStack == NULL){
        return -1;
    }
    copyState(&newStack, linkedList.current.pcbEntry.stack);
    entry.stack = newStack;
    entry.name = linkedList.current.pcbEntry.name;
    entry.foreground = linkedList.current.pcbEntry.foreground;
    entry.priority = linkedList.current.pcbEntry.priority;
    entry.state = linkedList.current.pcbEntry.state;
    
    // Should be other id
    uint64_t parentId = linkedList.current.pcbEntry.id;
    entry.id = unusedID();
    
    // Add this process to the scheduler
    add(entry);

    // If we are in the parent process
    if(linkedList.current.pcbEntry.id == parentId){
        return entry.id;
    }
    return 0;
}

int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp){
    ProcessControlBlockCDT currentProcess = linkedList.current.pcbEntry;
    

    // IT ONLY SETUPS THE PROCESS CORRECTLY IF ITS CALLED BY THE SYSCALL, U NEED TO SET THE STACK
    //BEFORE USING IT WITH INIT OR ANY FUNCTIONALITY THAT USES THIS FUNCTION WITHOUT USING THE SYSCALL
    return setProcess(process, argc, argv, rsp);
}

void init() {
    
    

}

// ----------- Implementación Round-Robin sin prioridad ----------------

void add(ProcessControlBlockCDT newEntry) {
    node_t *previous = NULL;
    node_t *current = &linkedList.head;
    while (current->next != NULL) {
            previous = current;
            current = current->next;
    }

    node_t *newNode = (node_t *)((uint64_t)current + sizeof(ProcessControlBlockCDT));

    newNode->pcbEntry.id = newEntry.id;
    newNode->pcbEntry.name = newEntry.name;
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
    node_t *head = &linkedList.head;
    node_t *current = &linkedList.current;

    // Si estamos en el último nodo, nos movemos al inicio.
    if (current->pcbEntry.state != READY && current->next == NULL) {
        current = &linkedList.head;
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
    linkedList.current = *current;

    return linkedList.current.pcbEntry;
}


ProcessControlBlockCDT remove(unsigned int id) {
    /* Si no lo encuentra => Loop infinito */

    node_t *current = &linkedList.current;
    if (current->pcbEntry.id != id && current->next == NULL) {
        current = &linkedList.head;
    }

    while (current->pcbEntry.id != id && current->next != NULL) {
        current = current->next;

        if (current->next == NULL) {
            if (current->pcbEntry.id != id) {
                current = &linkedList.head;
            }
        }
    }

    if (current->pcbEntry.id == id) {
        current->previous = current->next;
    }


}


ProcessControlBlockCDT* getEntry(uint32_t pid) {
    if (&linkedList.head == NULL || !exists(pid)) {
        return -1;
    }

    node_t *current = &linkedList.head;
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
    if (&linkedList.head == NULL) {
        return -1;
    }

    node_t *current = &linkedList.head;
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
    if (&linkedList.head == NULL) {
        return -1;
    }

    char founded = 0;
    node_t *current = &linkedList.head;
    founded = current->pcbEntry.id == pid ? 1 : 0; 

    while (current->next != NULL && !founded) {
        current = current->next;
        founded = current->pcbEntry.id == pid ? 1 : 0; 
    }

    return founded;
}

void copyState(uint64_t **targetStack, uint64_t *sourceStack);
void replaceProcess(processFunc process, int argvc, char *argv[], uint64_t **stack);

uint64_t scheduler() {
    /* La primera vez que se ejecuta el scheduler, debe iniciar el proceso init */
    if (firstTime) {
        init();
        firstTime = 0;
        return;
    }

    ProcessControlBlockCDT processToRun;

    /* Si fue llamado manualmente, mediante int 0x20, tenemos que
        revisar el estado de este proceso */
    if (linkedList.current.pcbEntry.state == EXITED) {
        node_t aux = linkedList.current;
        
        //Movemos el puntero al siguiente en ejecutar.
        processToRun = next(); 
        
        //Eliminamos el proceso actual (que está en estado EXITED) de la lista.
        remove(aux.pcbEntry.id);
    }

    /* If the process was BLOCKED or RUNNING, and scheduler function started, we do the same thing. 
        Get the next process with a READY state and run it.
    */
    processToRun = next();
    processToRun.state = RUNNING;
    switchProcess(processToRun.id);
    
    return processToRun.stack;
}

