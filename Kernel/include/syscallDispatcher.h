#ifndef _SYSCALL_DISPATCHER_H
#define _SYSCALL_DISPATCHER_H
#include <scheduler.h>

void set_SYSCALLS();

IPCBuffer *getSTDIN();
IPCBuffer *getSTDOUT();
IPCBuffer *getSTDERR();

#endif