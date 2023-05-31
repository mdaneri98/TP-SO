#ifndef PROCESS_MANAGEMENT_H
#define PROCESS_MANAGEMENT_H

#include <stdint.h>

// State for processes
typedef enum ProcessState { READY, RUNNING, BLOCKED, EXITED } ProcessState;
typedef enum BufferState { READ, WRITE, READ_WRITE, CLOSED } BufferState;

// May be util different BufferId for different pipes, FIXME: BufferType y BufferId for buffers
typedef enum BufferId {STDIN, STDOUT, STDERR, PIPE} BufferId;

#define PD_SIZE 32
#define PD_BUFF_SIZE 1024


typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;

typedef struct IPCBuffer IPCBuffer;

uint64_t getPCBNodeSize();

struct IPCBuffer{
    char buffer[PD_BUFF_SIZE];
    uint16_t bufferDim;
    uint16_t buffStart;
    uint32_t buffId;
    BufferState status;
    IPCBuffer * opositeEnd;
    ProcessControlBlockADT references[PD_SIZE];
};

typedef struct ProcessControlBlockCDT {
    uint32_t id;
    char foreground;
    ProcessState state;
    uint8_t priority;

    // Variables neccesary for computing priority scheduling
    uint8_t quantums;
    uint64_t agingInterval;
    uint64_t currentInterval;

    // All related to process hierarchy
    uint32_t parentId;
    uint32_t childsIds[PD_SIZE];
    
    // Each process will have its own buffers for reading and writing (since we don't have a filesystem)
    IPCBuffer readBuffer;
    IPCBuffer writeBuffer;

    // All the information necessary for running the stack of the process
    void *stack;
    void *baseStack;
    uint64_t stackSize;
    void *memoryFromMM;

    IPCBuffer *pdTable[PD_SIZE];
} ProcessControlBlockCDT;

#endif /* PROCESS_MANAGEMENT_H */