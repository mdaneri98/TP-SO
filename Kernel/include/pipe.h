#ifndef PIPE_H
#define PIPE_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "processManagment.h" //struct IPCBuffer, BufferState, ProcessState

#define ERROR -1;

/*Prototipes*/
uint64_t pipe(ProcessControlBlockADT process);
uint64_t readPipe(IPCBuffer * rEnd, uint64_t count, char * writeBuff);
void actualizeReadBuff(IPCBuffer * wEnd);

#endif /*PIPE_H*/