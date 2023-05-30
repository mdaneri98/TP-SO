#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <process.h>
#include "bufferManagment.h"

typedef struct PCBNodeCDT *PCBNodeADT;

void *scheduler(void *stack);
int sysFork();
int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp);
int sysKill(uint32_t pid);
void createInit();

ProcessControlBlockADT getEntry(uint32_t pid); //ESTE

PCBNodeADT getCurrentProcess();
ProcessControlBlockADT getCurrentProcessEntry();

IPCBuffer *getPDEntry(ProcessControlBlockADT entry, uint32_t pd); 
void setProcessState(ProcessControlBlockADT entry, ProcessState state);

#endif /* SCHEDULER_H */