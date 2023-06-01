#include <stdio.h>
#include <lib.h>
#include <phylo.h>
#include <filosopher.h>
#include <string.h>
#include <terminal.h>

#define BUFFER_MAX_LENGTH 250


/* FIXME: Puede haber un error al crear el nuevo argsv. */
int phylo(int argsc, char* argsv[]) {
    if (argsc <= 1) {
        printf("Hay menos de un argv\n");
        return 1;
    }

 //   int n = stringToInt(argsv[1]);

    printf("Se capturó: %s\n", argsv[0]);
    printf("Se capturó: %s\n", argsv[1]);
    /*
    for (int i = 0; i < n; i++) {
        if (_sysFork() == 0) {
            char* newArgsv[MAX_ARGS_COUNT];
            newArgsv[0] = "filosophers";
            for (int j = 1; j < argsc; j++) {
                stringCopy(newArgsv[j], BUFFER_MAX_LENGTH, argsv[j]);
            }
            _sysExecve(filosopher, argsc, newArgsv);
        } 
    }
    */
    // _wait();

    return 0;
}