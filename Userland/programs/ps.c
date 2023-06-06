
#include <ps.h>
#include <syscalls.h>
#include <stdio.h>


int ps(int argsc, char* argsv[]) {
    ProcessData data[256];

    int c = _sysPs(&data);
    for (int i = 0; i < c; i++) {  
        printf("ID: %d, Priority: %d, Stack: %x, Base: %x, Foreground: %d\n", data[i].id, data[i].priority, data[i].stack, data[i].baseStack, data[i].foreground);
    }

    return 0;
}