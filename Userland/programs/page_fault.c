
#include <stdio.h>
#include <syscalls.h>

extern void runPageFault(void);

int pageFault(int argsc, char* argsv[]) {
    printf("About to cause an exception...");
    runPageFault();
    return -1;
}