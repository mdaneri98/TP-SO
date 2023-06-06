// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <block.h>
#include <lib.h>

int block(int argsc, char* argsv[]) {
    if (argsc < 2) {
        return 1;
    }
    _sysChangeState(stringToInt(argsv[1]));

    return 0;
}
