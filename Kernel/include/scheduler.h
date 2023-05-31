#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <process.h>
#include <ps.h>
#include "bufferManagment.h"

typedef struct PCBNodeCDT *PCBNodeADT;

void *scheduler(void *stack);
int sysFork(void *currentProcessStack);
int sysExecve(processFunc process, int argc, char *argv[], void *rsp);
int sysPs(ProcessData data[]);

void createInit();

ProcessControlBlockADT getEntry(uint32_t pid); //ESTE

PCBNodeADT getCurrentProcess();
ProcessControlBlockADT getCurrentProcessEntry();
IPCBuffer *getPDEntry(ProcessControlBlockADT entry, uint32_t pd);
int setProcessState(ProcessControlBlockADT entry, ProcessState state);
int changePriority(uint32_t pid, unsigned int newPriority);
int changeState(uint32_t pid);
int hasOpenChilds(ProcessControlBlockADT entry);

#endif /* SCHEDULER_H */