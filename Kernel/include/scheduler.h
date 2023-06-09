#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <process.h>
#include <ps.h>
#include <bufferManagement.h>

// State for processes
typedef enum ProcessState { READY, RUNNING, BLOCKED, YIELDED, EXITED } ProcessState;

typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;
typedef struct PCBNodeCDT *PCBNodeADT;
typedef struct IPCBufferCDT *IPCBufferADT;

void *scheduler(void *stack);
int sysFork(void *currentProcessStack);
int sysExecve(processFunc process, int argc, char *argv[], void *rsp);
int sysPs(ProcessData data[]);

void createInit();

uint64_t getPCBNodeSize();
uint64_t getTSCFrequency();

ProcessControlBlockADT getEntry(uint32_t pid);
PCBNodeADT getCurrentProcess();
uint32_t getProcessId(ProcessControlBlockADT process);
ProcessControlBlockADT getForegroundProcess();
ProcessControlBlockADT getCurrentProcessEntry();
IPCBufferADT getPDEntry(ProcessControlBlockADT entry, uint32_t pd);
uint32_t getCurrentProcessPid();
ProcessState getProcessState(ProcessControlBlockADT process);
int setProcessState(ProcessControlBlockADT entry, ProcessState state);
int changePriority(uint32_t pid, unsigned int newPriority);
int changeState(uint32_t pid);
int hasOpenChilds(ProcessControlBlockADT entry);
void removeFromPDs(ProcessControlBlockADT process, IPCBufferADT buffToRemove);
void setProcessToForeground(ProcessControlBlockADT process);
void setProcessToBackground(ProcessControlBlockADT process);
void *sysMalloc(ProcessControlBlockADT process, uint64_t size);
void *sysRealloc(ProcessControlBlockADT process, void *toRealloc, uint64_t size);
void sysFree(ProcessControlBlockADT process, void *toFree);
int dupPd(ProcessControlBlockADT process, uint64_t oldPd, uint64_t newPd);
void sysClosePd(ProcessControlBlockADT process, IPCBufferADT toClose, uint32_t pd);
void setProcessPd(ProcessControlBlockADT process, IPCBufferADT buffer, uint64_t index);
int isInForeground(ProcessControlBlockADT process);
int isManualBlocked(ProcessControlBlockADT process);

void sysUnBlock(ProcessControlBlockADT process);
void sysBlock(ProcessControlBlockADT process);

int isBlocked(ProcessControlBlockADT process);
int isReady(ProcessControlBlockADT process);

#endif /* SCHEDULER_H */