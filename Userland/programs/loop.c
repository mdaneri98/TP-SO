/*
     Imprime su ID con un saludo cada una determinada cantidad de segundos.
*/

//sleep y funcion getpid
#include <syscalls.h>
#include <stdio.h>
#include <string.h>

#include <loop.h> //actualizar include path

#define LOOP_TIME 5000

int loop(int argsc, char* argsv[]) {
    //hasta que se presione ctrl+c
    while(1){
        printf("Process %d running\n", _getPid());
        _sleep(LOOP_TIME);
    }
}