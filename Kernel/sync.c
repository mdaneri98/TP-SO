#include "memoryManager.h"
#include <stdio.h>
#include <string.h>
#include <sync.h>
#include <semaphore.h>


#define MAX_SEM_QUEUE_SIZE 50

/*
my_sem_open(SEM_ID, 1)  "sem"
my_sem_wait(SEM_ID);    "sem"
my_sem_post(SEM_ID);    "sem"
my_sem_close(SEM_ID);   "sem"
*/

typedef struct {
    char * name;
    uint64_t currentValue;
    uint64_t mutex;
}semaphore_t;

typedef struct node_t {
    semaphore_t semaphore;
    struct node_t * nextNode;
} node_t;


typedef struct{
    node_t*head;
    node_t*tail;
}queue;

queue semaphoreQueue;


semaphore_t nodes[MAX_SEM_QUEUE_SIZE];
semaphore_t sem;

uint64_t* semOpen(char* semId, uint64_t currentValue){
    sem.name = semId; 
    sem.currentValue = currentValue;

    if(semaphoreQueue.head == NULL){
        node_t* newNode = malloc(sizeof(node_t)); 
        semaphoreQueue.head = newNode;
        semaphoreQueue.tail = newNode;
        newNode->semaphore.name = semId;
        newNode->semaphore.currentValue = currentValue;
        newNode->semaphore.mutex = 0;
        newNode->nextNode = NULL;

        return &newNode->semaphore.currentValue;
    } else {
        node_t* auxNode = semaphoreQueue.head;

        while (auxNode->nextNode != NULL) {
            /* Chequeamos que ninguno de los semáforos tenga el mismo nombre, o en caso que coincidan, devolvemos el currentValue del semáforo. */
            if(stringCompare(auxNode->semaphore.name, semId)){
                return &auxNode->semaphore.currentValue;
            }
            auxNode = auxNode->nextNode;
        }

        /* Si no había un semáforo con el nombre dado, lo creamos. */
        node_t* newNode = malloc(sizeof(node_t));
        semaphoreQueue.tail->nextNode = newNode;
        newNode->semaphore.name = semId;
        newNode->semaphore.currentValue = currentValue;
        newNode->semaphore.mutex = 0;
        newNode->nextNode = NULL;
        

        return &newNode->semaphore.currentValue;
    }

    return -1;
}

uint64_t semWait(char*semId) {
    node_t* auxNode = semaphoreQueue.head;

    while (auxNode->nextNode != NULL) {
        /* Chequeamos que ninguno de los semáforos tenga el mismo nombre, o en caso que coincidan, devolvemos el currentValue del semáforo. */
        if(stringCompare(auxNode->semaphore.name, semId)){
            uint64_t* currentValue = auxNode->semaphore.currentValue;
            uint64_t* mutex = auxNode->semaphore.mutex;
            semLock(currentValue, mutex);
            return 0;
        }
        auxNode = auxNode->nextNode;
    }
    return -1;  /* El semáforo con semId no se encontraba en la lista. */
}

uint64_t semPost(char*semId){
    node_t* auxNode = semaphoreQueue.head;

    while (auxNode->nextNode != NULL) {
        /* Chequeamos que ninguno de los semáforos tenga el mismo nombre, o en caso que coincidan, devolvemos el currentValue del semáforo. */
        if(stringCompare(auxNode->semaphore.name, semId)){
            uint64_t* currentValue = auxNode->semaphore.currentValue;
            semUnLock(currentValue);
            return 0;
        }
        auxNode = auxNode->nextNode;
    }
    return -1;  /* El semáforo con semId no se encontraba en la lista. */
}

uint64_t semClose(char*semId){
    
    return 1;
}