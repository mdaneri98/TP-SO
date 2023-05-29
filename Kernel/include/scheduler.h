#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <process.h>
#include <ps.h>

// States for processes.
typedef enum ProcessState { READY, RUNNING, BLOCKED, EXITED } ProcessState;

typedef enum BufferState { READ, WRITE, READ_WRITE, CLOSED } BufferState;

#define PD_SIZE 32
#define PD_BUFF_SIZE 1024

typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;

typedef struct PCBNodeCDT *PCBNodeADT;
typedef struct IPCBuffer{
    char buffer[PD_BUFF_SIZE];
    uint16_t bufferDim;
    uint32_t buffId;
    BufferState status;
    ProcessControlBlockADT references[PD_SIZE];
} IPCBuffer;

void *scheduler(void *stack);
int sysFork(void *currentProcessStack);
int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp);
int sysKill(uint32_t pid);
int sysPs(ProcessData data[]);

void createInit();
ProcessControlBlockADT getEntry(uint32_t pid);
PCBNodeADT getCurrentProcess();
ProcessControlBlockADT getCurrentProcessEntry();
IPCBuffer *getPDEntry(ProcessControlBlockADT entry, uint32_t pd);
void setProcessState(ProcessControlBlockADT entry, ProcessState state);
int killProcess(uint32_t pid);
int changePriority(uint32_t pid, unsigned int newPriority);
int changeState(uint32_t pid);
int hasOpenChilds(ProcessControlBlockADT entry);

#endif /* SCHEDULER_H */