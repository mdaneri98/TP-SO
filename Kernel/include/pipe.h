#ifndef PIPE_H
#define PIPE_H

#include <stdint.h>
#include <scheduler.h>
#include <bufferManagement.h>

#define ERROR -1;

/*Prototipes*/
int openPipe(ProcessControlBlockADT process, int pipeFds[2]);
void closePipe(IPCBufferADT bufferEnd);
uint64_t readPipe(IPCBufferADT rEnd, char *writeBuff, uint64_t count);
uint64_t writePipe(IPCBufferADT wEnd, char *dataToRead, uint64_t count);

#endif /*PIPE_H*/