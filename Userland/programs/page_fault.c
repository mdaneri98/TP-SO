// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdio.h>
#include <syscalls.h>

extern void runPageFault(void);

int pageFault(int argsc, char* argsv[]) {
    printf("About to cause an exception...");
    runPageFault();
    return -1;
}