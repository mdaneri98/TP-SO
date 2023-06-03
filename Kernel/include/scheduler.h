#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <process.h>
#include <ps.h>
#include <bufferManagement.h>

// State for processes
typedef enum ProcessState { READY, RUNNING, BLOCKED, EXITED } ProcessState;

typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;

typedef struct PCBNodeCDT *PCBNodeADT;

typedef struct IPCBufferCDT *IPCBufferADT;

void *scheduler(void *stack);
int sysFork(void *currentProcessStack);
int sysExecve(processFunc process, int argc, char *argv[], void *rsp);
int sysPs(ProcessData data[]);

void createInit();

uint64_t getPCBNodeSize();

ProcessControlBlockADT getEntry(uint32_t pid);
;
PCBNodeADT getCurrentProcess();
ProcessControlBlockADT getCurrentProcessEntry();
IPCBufferADT getPDEntry(ProcessControlBlockADT entry, uint32_t pd);
int setProcessState(ProcessControlBlockADT entry, ProcessState state);
int changePriority(uint32_t pid, unsigned int newPriority);
int changeState(uint32_t pid);
int hasOpenChilds(ProcessControlBlockADT entry);
void removeFromPDs(ProcessControlBlockADT process, IPCBufferADT buffToRemove);

#endif /* SCHEDULER_H */