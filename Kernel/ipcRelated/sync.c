#include <memoryManager.h>
#include <sync.h>
#include <semaphore.h>
#include <string.h>
#include <constants.h>

typedef struct {
    char * name;
    uint64_t currentValue;
    uint64_t mutex;
}semaphore_t;

typedef struct semNode {
    semaphore_t semaphore;
    struct semNode * nextNode;
} semNode;


typedef struct{
    semNode *head;
    semNode *tail;
}queue;


queue semaphoreQueue;

semaphore_t sem;

semaphore_t * getSemById(char * semId);

uint64_t semOpen(char* semId, uint64_t currentValue){
    sem.name = semId; 
    sem.currentValue = currentValue;

    if(semaphoreQueue.head == NULL){
        semNode* newNode = allocSemaphore(); 
        semaphoreQueue.head = newNode;
        semaphoreQueue.tail = newNode;
        newNode->semaphore.name = semId;
        newNode->semaphore.currentValue = currentValue;
        newNode->semaphore.mutex = 0;
        newNode->nextNode = NULL;

        return 0;
        //return &newNode->semaphore.currentValue;
    } else {
        semNode* auxNode = semaphoreQueue.head;

        while (auxNode->nextNode != NULL) {
            /* Chequeamos que ninguno de los semáforos tenga el mismo nombre, o en caso que coincidan, devolvemos el currentValue del semáforo. */
            if(stringCompare(auxNode->semaphore.name, semId)){
                return 0;
                //return &auxNode->semaphore.currentValue;
            }
            auxNode = auxNode->nextNode;
        }

        /* Si no había un semáforo con el nombre dado, lo creamos. */
        semNode* newNode = allocSemaphore();
        semaphoreQueue.tail->nextNode = newNode;
        newNode->semaphore.name = semId;
        newNode->semaphore.currentValue = currentValue;
        newNode->semaphore.mutex = 0;
        newNode->nextNode = NULL;
        

        //return &newNode->semaphore.currentValue;
        return 0;
    }
    return -1;
}

uint64_t semWait(char*semId) {
    semaphore_t * sem = getSemById(semId);
    if(sem == NULL){
        return -1;
    }
    _semLock(&sem->currentValue, &sem->mutex);
    return 0;
}

uint64_t semPost(char*semId){
    semaphore_t * sem = getSemById(semId);
    if(sem == NULL){
        return -1;      /* El semáforo con semId no se encontraba en la lista. */
    }
    _semUnlock(&sem->currentValue, &sem->mutex);
    return 0;
}

uint64_t semClose(char*semId){
    semNode* auxNode = semaphoreQueue.head;
    while (auxNode->nextNode != NULL && !stringCompare(auxNode->nextNode->semaphore.name, semId)) {
        auxNode = auxNode->nextNode;
    } 
    if(auxNode->nextNode == NULL){
        return -1;
    }
    auxNode->nextNode = auxNode->nextNode->nextNode;
    freeSemaphore(auxNode->nextNode);
    return 1;
}

semaphore_t * getSemById(char * semId){
    semNode* auxNode = semaphoreQueue.head;
    if (auxNode != NULL && stringCompare(auxNode->semaphore.name, semId) == 0) {
        return &auxNode->semaphore;
    }
    while(auxNode->nextNode != NULL){
        auxNode = auxNode->nextNode;
        if (stringCompare(auxNode->semaphore.name, semId)) {
            return &auxNode->semaphore;
        }
    }
    return NULL;
}

uint64_t getSemNodeSize(){
    return sizeof(semNode);
}