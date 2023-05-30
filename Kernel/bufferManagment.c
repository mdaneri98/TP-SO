#ifndef BUFFER_MANAGMENT_H
#define BUFFER_MANAGMENT_H

#include "bufferManagment.h"

IPCBuffer stdin;
IPCBuffer stdout;
IPCBuffer stderr;

uint64_t writeOnBuffer(IPCBuffer * wEnd, char * dataToWrite, uint64_t count){
    //solo me copia lo que entra en el buffer de write
    if(wEnd->status == WRITE || wEnd->status == READ_WRITE ){
        uint64_t wIndex;
        uint64_t bytesWritten = 0;
        while(wEnd->bufferDim < PD_BUFF_SIZE && bytesWritten < count){
            wIndex=(wEnd->buffStart + wEnd->bufferDim)%PD_BUFF_SIZE;
            wEnd->buffer[wIndex] = dataToWrite[bytesWritten++];
            wEnd->bufferDim++;
        }
        return bytesWritten;
    } else {
        return 0;
    }
}

static void removeReferences(IPCBuffer *pdBuffer, uint32_t pid){
    for(int i=0; i<PD_SIZE ;i++){
        if(pdBuffer->references[i] != NULL && pdBuffer->references[i]->id == pid){
            pdBuffer->references[i] == NULL;
        }
    }
}

IPCBuffer *getSTDIN(){
    return &stdin;
}
IPCBuffer *getSTDOUT(){
    return &stdout;
}
IPCBuffer *getSTDERR(){
    return &stderr;
}

#endif /*BUFFER_MANAGMENT_H*/