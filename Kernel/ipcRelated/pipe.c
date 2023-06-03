#include <pipe.h>
#include <bufferManagement.h>
#include <memoryManager.h>
#include <scheduler.h>

#define NULL (void *)0

static void updateWriteEnd(IPCBufferADT wEnd);
static void updateOppositeEnd(IPCBufferADT bufferEnd);

int openPipe(ProcessControlBlockADT process, int pipeFds[2]){
    IPCBufferADT pipe[2] = { NULL };
    int dim = 0;
    for(int i=0; i < PD_SIZE && dim < 2 ; i++){
        pipe[dim] = getPDEntry(process, i);
        if(pipe[dim] == NULL){
            pipe[dim] = createEmptyBuffer();
            if(dim == 0){
                setBufferId(pipe[dim], PIPE);
                setBufferState(pipe[dim], READ);
            } else{
                setBufferId(pipe[dim], PIPE);
                setBufferState(pipe[dim], WRITE);
            }
            pipeFds[dim] = i;
            dim++;
        }
    }
    if(dim < 2){
        if(pipe[0] != NULL){
            destroyBuffer(pipe[0]);
        }
        if(pipe[1] != NULL){
            destroyBuffer(pipe[1]);
        }
        return ERROR;
    }

    setBufferOppositeEnd(pipe[0], pipe[1]);
    setBufferOppositeEnd(pipe[1], pipe[0]);

    dim = 0;
    ProcessControlBlockADT aux;
    for(int i=0; i < PD_SIZE && i < 2 ;i++){
        aux = getReferenceByIndex(pipe[dim], i);
        if(aux == NULL){
            setReferenceByIndex(pipe[dim], process, i);
            dim++;
        }
    }
    if(dim < 2){
        destroyBuffer(pipe[0]);
        destroyBuffer(pipe[1]);
        return ERROR;
    }
    return 0;
}

//copies al the buffered data from the wEnd pipe to the rEnd pipe, depending the space available in the rEnd
static void updateOppositeEnd(IPCBufferADT bufferEnd){
    if(bufferEnd == NULL){
        return;
    }
    uint64_t bytesWritten;
    char *auxBuff[BUFF_SIZE] = { 0 };
    bytesWritten = copyFromBuffer(auxBuff, bufferEnd, BUFF_SIZE);
    if(bytesWritten != 0){
        IPCBufferADT otherEnd = getBufferOppositeEnd(bufferEnd);
        copyToBuffer(otherEnd, auxBuff, bytesWritten);
        setBufferReferencesReady(otherEnd);
    }
    return;
}

//copies the count given from the rEnd to the writeBuff
uint64_t readPipe(IPCBufferADT rEnd, char *buffToFill, uint64_t count){
    BufferState buffState = getBufferState(rEnd);
    if(rEnd != NULL && buffToFill != NULL && count > 0 && getBufferDim(rEnd) > 0 && (buffState == READ || buffState == READ_WRITE)){
        uint64_t bytesWritten = readBuffer(rEnd, buffToFill, count);

        updateOppositeEnd(getBufferOppositeEnd(rEnd));
        return bytesWritten;
    }
    return 0;
}

uint64_t writePipe(IPCBufferADT wEnd, char *dataToRead, uint64_t count){
    BufferState buffState = getBufferState(wEnd);
    if(wEnd != NULL && dataToRead != NULL && count > 0 && getBufferDim(wEnd) < BUFF_SIZE - 1  && (buffState == WRITE || buffState == READ_WRITE)){
        uint64_t bytesWritten = writeBuffer(wEnd, dataToRead, count);
        updateOppositeEnd(getBufferOppositeEnd(wEnd));
        return bytesWritten;
    }
    return 0;
}

void closePipe(IPCBufferADT bufferEnd){
    for(int i=0; i < PD_SIZE ;i++){
        setReferenceByIndex(bufferEnd, NULL, i);
    }
    IPCBufferADT otherEnd = getBufferOppositeEnd(bufferEnd);
    if(getBufferState(otherEnd) == CLOSED){
        destroyBuffer(bufferEnd);
        destroyBuffer(otherEnd);
    }
}

/*
● Crear y abrir pipes. 
● Leer y escribir de un pipe (notar que debe ser transparente para un proceso leer o
escribir de un pipe o de la terminal)
que pasa si otro proceso quiere escribir en el pipe? hay que bloquear la escritura y la lectura
*/

/*
en el buff to write habria que cambiar 
agregarle al buff un bufferType y poner si es un W-END-PIPE o R-END-PIPE
*/