#include <stdint.h>
#include <bufferManagement.h>
#include <memoryManager.h>

#define TRUE 1
#define FALSE 0

typedef struct IPCBufferCDT{
    char buffer[BUFF_SIZE];
    uint16_t bufferDim;
    uint16_t cursor;
    BufferId buffId;
    BufferState state;
    IPCBufferADT oppositeEnd;
    ProcessControlBlockADT references[PD_SIZE];
} IPCBufferCDT;

IPCBufferCDT stdin;
IPCBufferCDT stdout;
IPCBufferCDT stderr;

static int checkReferences(IPCBufferADT buffer);

void initStandardBuffers(){
    for(int i=0; i < PD_SIZE ;i++){
        stdin.references[i] = NULL;
        stdout.references[i] = NULL;
        stderr.references[i] = NULL;
    }
    for(int i=0; i < BUFF_SIZE ;i++){
        stdin.buffer[i] = '\0';
        stdout.buffer[i] = '\0';
        stderr.buffer[i] = '\0';
    }
    stdin.bufferDim = 0;
    stdout.bufferDim = 0;
    stderr.bufferDim = 0;

    stdin.cursor = 0;
    stdout.cursor = 0;
    stderr.cursor = 0;

    stdin.buffId = STDIN;
    stdout.buffId = STDOUT;
    stderr.buffId = STDERR;

    stdin.state = READ;
    stdout.state = WRITE;
    stderr.state = WRITE;
}

uint64_t writeBuffer(IPCBufferADT buffer, char *dataToWrite, uint64_t count){
    if(buffer != NULL && dataToWrite != NULL && count > 0 && buffer->bufferDim < BUFF_SIZE - 1  && (buffer->state == WRITE || buffer->state == READ_WRITE)){
        uint64_t bytesWritten = 0;
        while(buffer->bufferDim < BUFF_SIZE && bytesWritten < count){
            buffer->buffer[buffer->cursor + buffer->bufferDim % BUFF_SIZE] = dataToWrite[bytesWritten++];
            buffer->bufferDim++;
        }
        return bytesWritten;
    }
    return 0;
}

uint64_t readBuffer(IPCBufferADT rEnd, char *buffToFill, uint64_t count){
    if(rEnd != NULL && buffToFill != NULL && count > 0 && rEnd->bufferDim > 0 && (rEnd->state == READ || rEnd->state == READ_WRITE)){
        uint64_t bytesRead = 0;
        while(rEnd->bufferDim > 0 && bytesRead < count){
            buffToFill[bytesRead++] = rEnd->buffer[rEnd->cursor % BUFF_SIZE];
            rEnd->cursor++;
            rEnd->bufferDim--;
        }
        return bytesRead;
    }
    return 0;
}

uint64_t copyFromBuffer(char *toWrite, IPCBufferADT buffer, uint64_t count){
    if(toWrite != NULL && buffer != NULL && count > 0 && buffer->bufferDim > 0){
        uint64_t bytesWritten = 0;
        while(buffer->bufferDim > 0 && bytesWritten < count){
            toWrite[bytesWritten++] = buffer->buffer[buffer->cursor % BUFF_SIZE];
            buffer->cursor++;
            buffer->bufferDim--;
        }
        return bytesWritten;
    }
    return 0;
}

uint64_t copyToBuffer(IPCBufferADT buffer, char *toCopy, uint64_t count){
    if(toCopy != NULL && buffer != NULL && count > 0 && buffer->bufferDim < BUFF_SIZE - 1){
        uint64_t bytesWritten = 0;
        while(buffer->bufferDim < BUFF_SIZE && bytesWritten < count){
            buffer->buffer[buffer->cursor + buffer->bufferDim % BUFF_SIZE] = toCopy[bytesWritten++];
            buffer->bufferDim++;
        }
        return bytesWritten;
    }
    return 0;
}

ProcessControlBlockADT getReferenceByIndex(IPCBufferADT buffer, uint32_t index){
    if(index < PD_SIZE){
        return buffer->references[index];
    }
    return NULL;
}

void setReferenceByIndex(IPCBufferADT buffer, ProcessControlBlockADT toSet, uint32_t index){
    if(index < PD_SIZE){
        buffer->references[index] = toSet;
    }
    checkReferences(buffer);
}

static int checkReferences(IPCBufferADT buffer){
    if(buffer->buffId == PIPE){
        for(int i=0; i<PD_SIZE ;i++){
            if(buffer->references[i] != NULL){
                return FALSE;
            }
        }
        setBufferState(buffer, CLOSED);
        return TRUE;
    }
}

void setBufferState(IPCBufferADT buffer, BufferState status){
    buffer->state = status;
}

void setBufferId(IPCBufferADT buffer, BufferId id){
    buffer->buffId = id;
}

BufferId getBufferId(IPCBufferADT buffer){
    return buffer->buffId;
}

void setBufferOppositeEnd(IPCBufferADT buffer, IPCBufferADT buffEnd){
    buffer->oppositeEnd = buffEnd;
}

IPCBufferADT getBufferOppositeEnd(IPCBufferADT buffer){
    return buffer->oppositeEnd;
}

uint16_t getBufferDim(IPCBufferADT buffer){
    return buffer->bufferDim;
}

void clearBuffer(IPCBufferADT buffer){
    for(int i=0; i<BUFF_SIZE ;i++){
        buffer->buffer[i] = '\0';
    }
    buffer->bufferDim = 0;
    buffer->cursor = 0;
}

IPCBufferCDT *getSTDIN(){
    return &stdin;
}
IPCBufferCDT *getSTDOUT(){
    return &stdout;
}
IPCBufferCDT *getSTDERR(){
    return &stderr;
}

uint64_t getIPCBufferSize(){
    return sizeof(IPCBufferCDT);
}

BufferState getBufferState(IPCBufferADT buffer){
    return buffer->state;
}

IPCBufferADT createEmptyBuffer(){
    IPCBufferADT toReturn = (IPCBufferADT) allocBuffer();
    clearBuffer(toReturn);

    for(int i=0; i < PD_SIZE ;i++){
        toReturn->references[i] = NULL;
    }
    toReturn->oppositeEnd = NULL;

    return toReturn;
}

void destroyBuffer(IPCBufferADT buffToDestroy){
    if(!checkReferences(buffToDestroy)){
        for(int i=0; i < PD_SIZE ;i++){
            if(buffToDestroy->references[i] != NULL){
                removeFromPDs(buffToDestroy->references[i], buffToDestroy);
            }
        }
    }
    freeBuffer((void *)buffToDestroy);
}

void setBufferReferencesReady(IPCBufferADT buffer){
    for(int i=0; i<PD_SIZE ;i++){
        if(buffer->references[i] != NULL){
            setProcessState(buffer->references[i], READY);
        }
    }
}