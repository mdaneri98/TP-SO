// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <syscalls.h>

extern void runInvalidOpcode(void);

int invalidOpCode(int argsc, char* argsv[]) {
    runInvalidOpcode();
    return -1;
}

