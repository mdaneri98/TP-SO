#include <lib.h>
#include <phylo.h>
#include <filosopher.h>
#include <string.h>
#include <shared_variables.h>

#define BUFFER_MAX_LENGTH 250
#define MAX_PHILOSOPHERS 12

sem_t* s[MAX_PHILOSOPHERS];
phylState state[MAX_PHILOSOPHERS];
char semNames[MAX_PHILOSOPHERS][BUFFER_MAX_LENGTH];
char ansiArt[MAX_PHILOSOPHERS];
sem_t* mutexSem;

/* FIXME: Puede haber un error al crear el nuevo argsv. */
int phylo(int argsc, char* argsv[]) {
    if (argsc <= 1) {
        printf("Hay menos de un argv\n");
        return 1;
    }

    int n = stringToInt(argsv[1]);

    for (int i = 0; i < n; i++) {
        state[i] = HUNGRY;
        ansiArt[i] = '.';
        
        // Crear el string utilizando el valor de 'i'
        stringFormat(semNames[i], BUFFER_MAX_LENGTH, "filoSem_%d", i);
        s[i] = _sysSemOpen(semNames[i], 1);
    }

    char semName[] = "mutex";
    mutexSem = _sysSemOpen(semName, 0);

    int i;
    for (i = 0; i < n; i++) {
        if (_sysFork() == 0) {

            char** newArgsv = malloc(sizeof(char*) * 3);
            for (int j = 0; j < 3; j++) {
                newArgsv[j] = malloc(sizeof(char) * BUFFER_MAX_LENGTH);
            }
            stringCopy(newArgsv[0], BUFFER_MAX_LENGTH, "philosopher");
            stringCopy(newArgsv[2], BUFFER_MAX_LENGTH, argsv[1]);        
            
            char aux[BUFFER_MAX_LENGTH] = { '0' };
            stringFormat(aux, BUFFER_MAX_LENGTH, "%d", i);
            stringCopy(newArgsv[1], BUFFER_MAX_LENGTH, aux);

            _sysExecve(filosopher, argsc, newArgsv);
        } 
    }
    
    _wait();

    return 0;
}