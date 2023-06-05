

#include <syscalls.h>


int invalidOpCode(int argsc, char* argsv[]) {
    printf("About to cause an exception...\n");
    _sleep(1500);
    runInvalidOpcode();
}

