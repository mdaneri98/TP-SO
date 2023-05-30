#include <stdio.h>
#include <lib.h>

/* --------------- Filósofos. --------------- */

typedef int sem_t;



static void phylo(int argsc, char* argsv[]) {
    int n = 0;
    
    int state[n];
    sem_t mutex = _sysSemOpen("mutex", 1);

    /* FIXME: Falta inicializar cada semáforo. */
    sem_t s[n];

    for (int i = 0; i < n; i++) {
        if (_sysFork() == 0) {
            _sysExecve()
        }    
    
    }
}