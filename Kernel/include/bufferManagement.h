#ifndef BUFFER_MANAGEMENT_H
#define BUFFER_MANAGEMENT_H

#include <stdint.h>
#include <processManagement.h>

#define NULL (void *)0

uint64_t writeOnBuffer(IPCBuffer * wEnd, char * dataToWrite, uint64_t count);
IPCBuffer *getSTDIN();
IPCBuffer *getSTDOUT();
IPCBuffer *getSTDERR();

#endif /* BUFFER_MANAGEMENT_H */