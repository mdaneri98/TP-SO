
#include <stdio.h>
#include <syscalls.h>

int pageFault(int argsc, char* argsv[]) {
    printf("About to cause an exception...");
    runPageFault();
}