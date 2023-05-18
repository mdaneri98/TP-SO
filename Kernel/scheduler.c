#include <scheduler.h>
#include <stdlib.h>
#include <freeListMemoryManager.h>  //FIXME: Debería ser uno general.

typedef struct ProcessControlBlockCDT {
    char* name;
    int id;
    int priority;
    int stack;
    char foreground;
    ProcessState state;
    uint64_t stack;
    uint64_t base;
    uint64_t counter;

} ProcessControlBlockCDT;

typedef struct node {
    struct node *next;
    struct node *prev;
    ProcessControlBlockCDT pcbEntry;
} node_t;

typedef struct list {
    node_t head;
} list_t;

list_t linkedList;

uint32_t entriesCount = 0;


ProcessControlBlockCDT createInit() {
    node_t *node = PCB_TABLE;

    ProcessControlBlockCDT *pcbEntry = &(node->pcbEntry);
    pcbEntry->name = "init"; //String en zona de datos.
    pcbEntry->id = 0;
    pcbEntry->priority = 0;
    pcbEntry->foreground = 1;
    pcbEntry->state = RUNNING;
}



void startScheduler() {
    linkedList.head.next = NULL;
    linkedList.head.prev = NULL;
    

}

void add() {

    node_t *current = &linkedList.head;
    while (current->next != NULL) {
        current = current->next;
    }

    
    


}

void next() {

}


