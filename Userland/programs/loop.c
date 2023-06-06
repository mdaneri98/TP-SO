// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <syscalls.h>
#include <stdio.h>
#include <string.h>
#include <loop.h> 

#include <loop.h>

#define LOOP_TIME 30000

int loop(int argsc, char* argsv[]) {
    while(1){
        printf("Process %d running\n", _getPid());
        _sleep(LOOP_TIME);
    }
}