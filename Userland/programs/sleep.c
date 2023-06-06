// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <stdio.h>
#include <string.h>
#include <syscalls.h>

int sleep(int argc, char* argsv[]) {
    unsigned long millis = 0;
    if(argc == 1) {
        millis = 2000;
    } else {
        millis = stringToNum(argsv[1]);
        if(millis == 0){
            char *argErr = "You can enter only one number.";
            printf(argErr);
            putChar('\n');
            return -1;
        }
    }
    _sleep(millis);

    return 0;
}

