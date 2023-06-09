// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <lib.h>
#include <phylo.h>
#include <filosopher.h>
#include <string.h>
#include <shared_variables.h>


#define BUFFER_MAX_LENGTH 250
#define MAX_PHILOSOPHERS 12

char* s[MAX_PHILOSOPHERS];
phylState state[MAX_PHILOSOPHERS];
char semNames[MAX_PHILOSOPHERS][BUFFER_MAX_LENGTH];
char ansiArt[MAX_PHILOSOPHERS];
char* mutexSem;

/* Due to rbp's limitation, _fork uses the same local variables after execution, therefore, i must be global. */
int filosopherNumber;
int argNumber;
char aux[BUFFER_MAX_LENGTH] = { '0' };
char** newArgsv;


int phylo(int argsc, char* argsv[]) {
    if (argsc <= 1) {
        printf("Hay menos de un argv\n");
        return 1;
    }

    int n = stringToInt(argsv[1]);

    for (int i = 0; i < n; i++) {
        state[i] = HUNGRY;
        ansiArt[i] = '.';
        
        // Create the string using the value of 'i'.
        s[i] = malloc(BUFFER_MAX_LENGTH);
        for (int i = 0; i < BUFFER_MAX_LENGTH; i++)
            s[i] = 0x0;

        stringFormat(semNames[i], BUFFER_MAX_LENGTH, "filoSem_%d", i);
        stringCopy(s[i], BUFFER_MAX_LENGTH, semNames[i]);
        _sysSemOpen(s[i], 1);
    }

    _sysSemOpen(FILO_MUTEX, 0);

    for (filosopherNumber = 0; filosopherNumber < n; filosopherNumber++) {
        if (_sysFork() == 0) {
            newArgsv = malloc(sizeof(char*) * 3);
            for (argNumber = 0; argNumber < 3; argNumber++) {
                newArgsv[argNumber] = malloc(sizeof(char) * BUFFER_MAX_LENGTH);
            }
            stringCopy(newArgsv[0], BUFFER_MAX_LENGTH, "philosopher");
            stringCopy(newArgsv[2], BUFFER_MAX_LENGTH, argsv[1]);        
            
            stringFormat(aux, BUFFER_MAX_LENGTH, "%d", filosopherNumber);
            stringCopy(newArgsv[1], BUFFER_MAX_LENGTH, aux);

            _sysExecve(filosopher, argsc, newArgsv);
        } 
        _yield();
    }
    
    _wait();

    return 0;
}