#ifndef BUFFER_MANAGEMENT_H
#define BUFFER_MANAGEMENT_H

#include <stdint.h>
#include <scheduler.h>

typedef enum BufferState { READ, WRITE, READ_WRITE, CLOSED } BufferState;

// May be util different BufferId for different pipes, FIXME: BufferType y BufferId for buffers
typedef enum BufferId {STDIN, STDOUT, STDERR, PIPE} BufferId;

#define PD_SIZE 32
#define BUFF_SIZE 1024

typedef struct IPCBufferCDT *IPCBufferADT;

typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;

void initStandardBuffers();

IPCBufferADT createEmptyBuffer();
void destroyBuffer(IPCBufferADT buffToDestroy);

uint64_t writeBuffer(IPCBufferADT wEnd, char *dataToWrite, uint64_t count);
uint64_t readBuffer(IPCBufferADT rEnd, char *buffToFill, uint64_t count);
uint64_t copyFromBuffer(char *toCopy, IPCBufferADT buffer, uint64_t count);

void clearBuffer(IPCBufferADT buffer);
ProcessControlBlockADT getReferenceByIndex(IPCBufferADT pdBuffer, uint32_t index);
BufferState getBufferState(IPCBufferADT buffer);
BufferId getBufferId(IPCBufferADT buffer);
void setBufferOppositeEnd(IPCBufferADT buffer, IPCBufferADT buffEnd);
IPCBufferADT getBufferOppositeEnd(IPCBufferADT buffer);
uint16_t getBufferDim(IPCBufferADT buffer);

void setBufferReferencesReady(IPCBufferADT buffer);
uint64_t copyToBuffer(IPCBufferADT buffer, char *toCopy, uint64_t count);

int setReferenceByIndex(IPCBufferADT pdBuffer, ProcessControlBlockADT toSet, uint32_t index);
void setBufferState(IPCBufferADT buffer, BufferState state);
void setBufferId(IPCBufferADT buffer, BufferId id);
IPCBufferADT getSTDIN();
IPCBufferADT getSTDOUT();
IPCBufferADT getSTDERR();
uint64_t getIPCBufferSize();

#endif /* BUFFER_MANAGEMENT_H */