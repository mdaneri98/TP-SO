#include "pipe.h"

void actualizeReadBuff(IPCBuffer * wEnd);

uint64_t pipe(ProcessControlBlockADT process){
    int indexWEnd = 0;

    while(process->pdTable[indexWEnd]->status != CLOSED){
        indexWEnd++;
    }
    if(indexWEnd >= PD_SIZE - 1){
        return ERROR;
    }

    IPCBuffer * pipeWEnd = (process->pdTable[indexWEnd]);
    pipeWEnd->status = WRITE;
    pipeWEnd->buffStart = 0;
    pipeWEnd->bufferDim = 0;
    pipeWEnd->buffId = PIPE;

    int indexREnd = indexWEnd+1;
    while(process->pdTable[indexREnd]->status != CLOSED){
        indexREnd++;
    }
    
    IPCBuffer * pipeREnd = (process->pdTable[indexREnd]);
    pipeREnd->status = READ;
    pipeREnd->buffStart = 0;
    pipeREnd->bufferDim = 0;
    pipeREnd->buffId = PIPE;
    pipeREnd->opositeEnd = (process->pdTable[indexWEnd]);

    pipeWEnd->opositeEnd = (process->pdTable[indexREnd]);

    for(int i=0; i<PD_BUFF_SIZE; i++){
        pipeWEnd->buffer[i] = '\0';
        pipeREnd->buffer[i] = '\0';
    } 
    for(int i=0; i<PD_SIZE ;i++){
        pipeWEnd->references[i] = NULL;
        pipeREnd->references[i] = NULL;
    }
    return 0;
}

//copies al the buffered data from the wEnd pipe to the rEnd pipe, depending the space available in the rEnd
void actualizeReadBuff(IPCBuffer * wEnd){
    uint64_t readPipeIndex;
    while(wEnd->opositeEnd->bufferDim < PD_BUFF_SIZE && wEnd->bufferDim > 0){
        readPipeIndex = (wEnd->opositeEnd->buffStart + wEnd->opositeEnd->bufferDim)%PD_BUFF_SIZE;
        wEnd->opositeEnd->buffer[readPipeIndex] = wEnd->buffer[wEnd->buffStart];
        wEnd->buffer[wEnd->buffStart] = '\0';
        wEnd->buffStart = (wEnd->buffStart+1)%PD_BUFF_SIZE;
        wEnd->bufferDim--;
        wEnd->opositeEnd->bufferDim++;
    }
}

//copies the count given from the rEnd to the writeBuff
uint64_t readPipe(IPCBuffer * rEnd, uint64_t count, char * writeBuff){
    if(rEnd->status == READ || rEnd->status == READ_WRITE){
        uint16_t toWriteIndex = 0;

        while(toWriteIndex<count && rEnd->bufferDim > 0){
            writeBuff[toWriteIndex] = rEnd->buffer[rEnd->buffStart];
            rEnd->buffer[rEnd->buffStart] = '\0';
            rEnd->bufferDim--;
            rEnd->buffStart = (rEnd->buffStart+1)%PD_BUFF_SIZE;
            toWriteIndex++;
        }
        printf("rEnd->readEnd: %p\n", rEnd->opositeEnd);
        actualizeReadBuff(rEnd->opositeEnd);
        return toWriteIndex;
    } else {
        return ERROR;
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
