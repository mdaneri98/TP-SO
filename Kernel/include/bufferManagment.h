#include "processManagment.h"

#define NULL (void *)0

uint64_t writeOnBuffer(IPCBuffer * wEnd, char * dataToWrite, uint64_t count);
void removeReferences(IPCBuffer *pdBuffer, uint32_t pid);
IPCBuffer *getSTDIN();
IPCBuffer *getSTDOUT();
IPCBuffer *getSTDERR();