#include <mem.h>
#include <syscalls.h>
#include <stdio.h>

int mem(int argsc, char* argsv[]) {

    printf("Memory State\nUsed memory: %d \n", _getUsedMemory() );
    printf("Free memory: %d \n", _getFreeMemory() );

    return 0;
}