#include <stdio.h>
#include <lib.h>
#include <phylo.h>
#include <filosopher.h>
#include <string.h>
#include <terminal.h>

#define BUFFER_MAX_LENGTH 250


/* FIXME: Puede haber un error al crear el nuevo argsv. */
int phylo(int argsc, char* argsv[]) {
    int n = stringToInt(argsv[1]);
    
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

    return 0;
}