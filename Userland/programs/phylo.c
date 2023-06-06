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

/* Por limitación de rbp, _fork usa las mismas variables locales luego de realizarse, por ende, i debe ser global.
 */
int filosopherNumber;
int argNumber;
char aux[BUFFER_MAX_LENGTH] = { '0' };
char** newArgsv;

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
        stringCopy(s[i], BUFFER_MAX_LENGTH, semNames[i]);
        _sysSemOpen(s[i], 1);
    }

    char semName[] = "mutex";
    mutexSem = _sysSemOpen(semName, 0);

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