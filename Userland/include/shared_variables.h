#ifndef SHARED_VARIABLES_H
#define SHARED_VARIABLES_H

#include <filosopher.h>

typedef int sem_t;

extern sem_t* s[]; 
extern phylState state[]; 
extern char ansiArt[]; 
extern sem_t* mutexSem;

#endif
