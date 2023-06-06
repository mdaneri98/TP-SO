
#include <syscalls.h>

extern void runDivzero(void);

int divZero(int argsc, char* argsv[]) {
    runDivzero();
    return -1;
}