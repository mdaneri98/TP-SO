

#define PCB_TABLE 0x0000000000010000

// States for processes.
typedef enum ProcessState { READY, RUNNING, BLOCKED } ProcessState;


typedef struct ProcessControlBlockCDT *ProcessControlBlockADT;