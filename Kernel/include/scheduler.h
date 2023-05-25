#ifndef SCHEDULER_H
#define SCHEDULER_H
#include <process.h>

// States for processes.
typedef enum ProcessState { READY, RUNNING, BLOCKED, EXITED } ProcessState;

typedef enum BufferState { READ, WRITE, READ_WRITE, CLOSED } BufferState;

#define PD_SIZE 32
#define PD_BUFF_SIZE 1024

typedef struct buffer_t{
    char buffer[PD_BUFF_SIZE];
    uint16_t bufferDim;
    uint32_t buffId;
    BufferState status;
    struct ProcessControlBlockCDT *references[PD_SIZE];
} buffer_t;


typedef struct ProcessControlBlockCDT {
    uint32_t id;
    uint16_t priority;
    char foreground;
    ProcessState state;
    
    buffer_t readBuffer;
    buffer_t writeBuffer;

    void *stack;
    void *baseStack;
    void *memoryFromMM;

    buffer_t *pdTable[PD_SIZE];
} ProcessControlBlockCDT;

typedef struct pcb_node {
    struct pcb_node *next;
    struct pcb_node *previous;
    ProcessControlBlockCDT pcbEntry;
} PCBNode;

void *scheduler(void *stack);
int sysFork();
int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp);
int sysKill(uint32_t pid);
int sysBlock(uint32_t pid);
void createInit();
ProcessControlBlockCDT *getEntry(uint32_t pid);
PCBNode *getCurrentProcess();

typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;
#endif /* SCHEDULER_H */