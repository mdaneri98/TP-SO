#include "memoryManager.h"
#include <stdio.h>
#include <string.h>


typedef struct {
    char * name;
    uint64_t currentValue;
    uint64_t mutex;
}semaphore_t;

typedef struct node_t node_t;

struct node_t{
    semaphore_t semaphore;
    node_t * nextNode;
};

typedef struct{
    node_t*head;
    node_t*tail;
}queue;


queue semaphoreQueue;

semaphore_t sem;

semaphore_t * getSemById(char * semId);

uint64_t semOpen(char* semId, uint64_t currentValue){
    sem.name = semId; 
    sem.currentValue = currentValue;

    if(semaphoreQueue.head == NULL){
        node_t* newNode = malloc(sizeof(node_t)); 
        semaphoreQueue.head = newNode;
        semaphoreQueue.tail = newNode;
        newNode->semaphore.name = semId;
        newNode->semaphore.currentValue = currentValue;
        newNode->nextNode = NULL;
    } else {
        node_t* auxNode = semaphoreQueue.head;

        while (auxNode->nextNode != NULL) {
            /* Chequeamos que ninguno de los semáforos tenga el mismo nombre, o en caso que coincidan, devolvemos el currentValue del semáforo. */
            if(strcmp(auxNode->semaphore.name, semId)){
                return auxNode->semaphore.currentValue;
            }
            auxNode = auxNode->nextNode;
        }

        /* Si no había un semáforo con el nombre dado, lo creamos. */
        node_t* newNode = malloc(sizeof(node_t));
        semaphoreQueue.tail->nextNode = newNode;
        newNode->semaphore.name = semId;
        newNode->semaphore.currentValue = currentValue;
        newNode->nextNode = NULL;

        return newNode->semaphore.currentValue;
    }
    return -1;
}

void semWait(char*semId){
    semaphore_t * sem = getSemById(semId);
    semLock(sem->currentValue, sem->mutex);
}

void semPost(char*semId){
    semaphore_t * sem = getSemById(semId);
    semUnlock(sem->currentValue, sem->mutex);
}

uint64_t semClose(char*semId){
    node_t* auxNode = semaphoreQueue.head;
    while (auxNode->nextNode != NULL && auxNode->nextNode->semaphore.name != semId) {
        auxNode = auxNode->nextNode;
    } 
    if(auxNode->nextNode == NULL){
        return -1;
    }
    auxNode->nextNode = auxNode->nextNode->nextNode;
    free(auxNode->nextNode);
    return 1;
}

semaphore_t * getSemById(char * semId){
    node_t* auxNode = semaphoreQueue.head;
    while(auxNode->nextNode != NULL && auxNode->semaphore.name != semId){
        auxNode = auxNode->nextNode;
    }
    if(auxNode->nextNode == NULL && auxNode->semaphore.name != semId){
        return NULL;
    } else {
        return &auxNode->semaphore;
    }
}