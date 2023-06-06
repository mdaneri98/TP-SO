
#include <syscalls.h>

extern void runInvalidOpcode(void);

int invalidOpCode(int argsc, char* argsv[]) {
    runInvalidOpcode();
    return -1;
}

