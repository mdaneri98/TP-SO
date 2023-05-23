#ifndef _SYSCALL_DISPATCHER_H
#define _SYSCALL_DISPATCHER_H
#include <scheduler.h>

void set_SYSCALLS();

buffer_t *getSTDIN();
buffer_t *getSTDOUT();
buffer_t *getSTDERR();

#endif