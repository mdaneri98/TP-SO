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