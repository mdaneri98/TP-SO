#include <process.h>


// States for processes.
typedef enum ProcessState { READY, RUNNING, BLOCKED, EXITED } ProcessState;

typedef struct ProcessControlBlockCDT {
    unsigned int id;
    unsigned int priority;
    char foreground;
    ProcessState state;
    uint64_t *stack;
    uint64_t *stackBase;
} ProcessControlBlockCDT;

typedef struct pcb_node {
    struct pcb_node *next;
    struct pcb_node *previous;
    ProcessControlBlockCDT pcbEntry;
} PCBNode;

void scheduler();
int sysFork();
int sysExecve(processFunc process, int argc, char *argv[], uint64_t rsp);
int sysKill(uint32_t pid);
int sysBlock(uint32_t pid);
void createInit();

typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;