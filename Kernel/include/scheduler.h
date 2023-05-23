#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <process.h>

#define READ 0
#define WRITE 1
#define READ_WRITE 2
#define CLOSED 3

#define PD_SIZE 32
#define PD_BUFF_SIZE 1024
// States for processes.
typedef enum ProcessState { READY, RUNNING, BLOCKED, EXITED } ProcessState;

typedef struct buffer_t{
    char buffer[PD_BUFF_SIZE];
    uint16_t bufferDim;
    uint32_t buffId;
    uint8_t status;
} buffer_t;

typedef struct ProcessControlBlockCDT {
    uint32_t id;
    uint16_t priority;
    char foreground;
    ProcessState state;
    
    buffer_t readBuffer;
    buffer_t writeBuffer;

    uint64_t *stack;
    uint64_t *stackBase;

    buffer_t *pdTable[PD_SIZE];
} ProcessControlBlockCDT;

typedef struct pcb_node {
    struct pcb_node *next;
    struct pcb_node *previous;
    ProcessControlBlockCDT pcbEntry;
} PCBNode;

uint64_t *scheduler();
int sysFork();
int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp);
int sysKill(uint32_t pid);
int sysBlock(uint32_t pid);
void createInit();

typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;
#endif /* SCHEDULER_H */