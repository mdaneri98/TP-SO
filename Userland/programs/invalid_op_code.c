
#include <syscalls.h>


int invalidOpCode(int argsc, char* argsv[]) {
    runInvalidOpcode();
    return -1;
}

