// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <nice.h>
#include <lib.h>

int nice(int argsc, char* argsv[]) {
    if (argsc < 2) {
        return 1;
    }
    _sysPriority(stringToInt(argsv[1]), stringToInt(argsv[2]));

    return 1;
}