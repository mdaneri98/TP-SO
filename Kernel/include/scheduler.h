#include <stdlib.h>
#include "process.h"

#define PCB_TABLE 0x50000

// States for processes.
typedef enum ProcessState { READY, RUNNING, BLOCKED, EXITED } ProcessState;

void scheduler();
int sysFork();
int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp);
int sysKill(uint32_t pid);
int sysBlock(uint32_t pid);
void createInit();

typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;