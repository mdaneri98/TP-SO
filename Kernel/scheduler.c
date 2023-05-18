#include <scheduler.h>
#include <stdlib.h>
#include <freeListMemoryManager.h>  //FIXME: Debería ser uno general.

typedef struct ProcessControlBlockCDT {
    char* name;
    unsigned int id;
    unsigned int priority;
    char foreground;
    ProcessState state;
    uint64_t stack;
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


ProcessControlBlockCDT createInit() {
    node_t *node = PCB_TABLE;

    ProcessControlBlockCDT *pcbEntry = &(node->pcbEntry);
    pcbEntry->name = "init"; //String en zona de datos.
    pcbEntry->id = 0;
    pcbEntry->priority = 0;
    pcbEntry->foreground = 1;
    pcbEntry->state = RUNNING;
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

    node_t *newNode = current + sizeof(ProcessControlBlockCDT);

    newNode->pcbEntry.id = newEntry.id;
    newNode->pcbEntry.name = newEntry.name;
    newNode->pcbEntry.priority = newEntry.priority;
    newNode->pcbEntry.stack = newEntry.stack;
    newNode->pcbEntry.state = newEntry.state;
    newNode->previous = current;

    current->next = newNode;
    newNode->next = NULL;
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



void scheduler() {
    /* La primera vez que se ejecuta el scheduler, debe iniciar el proceso init */
    if (firstTime) {
        init();
        firstTime = 0;
    }

    /* Si fue llamado manualmente, mediante int 0x20, tenemos que
        revisar el estado de este proceso */
    if (linkedList.current.pcbEntry.state == EXITED) {
        node_t aux = linkedList.current;
        
        //Movemos el puntero al siguiente en ejecutar.
        ProcessControlBlockCDT _ = next(); 
        
        //Eliminamos el proceso actual (que está en estado EXITED) de la lista.
        remove(aux.pcbEntry.id);
    }


    ProcessControlBlockCDT pcbEntry = next();
    
    //Falta

}

