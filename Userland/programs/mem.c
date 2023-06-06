#include <mem.h>
#include <syscalls.h>
#include <stdio.h>
#include <string.h>
#include <constants.h>

int mem(int argsc, char* argsv[]) {

    char memory[BUFFER_MAX_LENGTH] = { 0x0 };

    uintToBase(_getUsedMemory(), memory, 10);
    printf("Memory State\nUsed memory: %s \n", memory);

    for (int i = 0; i<BUFFER_MAX_LENGTH; i++)
        memory[i] = 0x0;

    uintToBase(_getFreeMemory(), memory, 10);
    printf("Free memory: %s \n", memory);

    return 0;
}