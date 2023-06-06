// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <stdio.h>
#include <syscalls.h>
#include <test_util.h>
#include <string.h>
#include <lib.h>
#include <constants.h>

#define SEM_ID "sem"
#define TOTAL_PAIR_PROCESSES 2

typedef int sem_t;

int64_t global; // shared memory

void slowInc(int64_t *p, int64_t inc) {
  uint64_t aux = *p;
  // _yield(); // This makes the race condition highly probable
  aux += inc;
  *p = aux;
}


int my_process_inc(int argc, char *argv[]) {
  uint64_t n;
  int8_t inc;
  int8_t use_sem;

  if (argc != 3)
    return -1;

  if ((n = satoi(argv[0])) <= 0)
    return -1;
  if ((inc = satoi(argv[1])) == 0)
    return -1;
  if ((use_sem = satoi(argv[2])) <= 0)
    return -1;


  if (use_sem)
    if (_sysSemOpen(SEM_ID, 1) == -1) {
      printf("test_sync: ERROR opening semaphore\n");
      return -1;
    }

  uint64_t i;
  for (i = 0; i < n; i++) {
    if (use_sem)
      _sysSemDown(SEM_ID);
    slowInc(&global, inc);
    if (use_sem)
      _sysSemUp(SEM_ID);
  }

  if (use_sem)
    _sysSemClose(SEM_ID);

   return 0;
}


int test_sync(int argc, char *argv[]) { //{n, use_sem, 0}
  
  if (argc != 3)
    return -1;


  char** argsvAux = malloc(sizeof(char*) * 4);
  argsvAux[0] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  argsvAux[1] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  argsvAux[2] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  stringCopy(argsvAux[0], BUFFER_MAX_LENGTH, argv[0]);
  stringCopy(argsvAux[1], BUFFER_MAX_LENGTH, "-1");
  stringCopy(argsvAux[2], BUFFER_MAX_LENGTH, argv[2]);
  argsvAux[3] = NULL;

  char** argsvAux2 = malloc(sizeof(char*) * 4);
  argsvAux[0] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  argsvAux[1] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  argsvAux[2] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  stringCopy(argsvAux2[0], BUFFER_MAX_LENGTH, argv[0]);
  stringCopy(argsvAux2[1], BUFFER_MAX_LENGTH, "-1");
  stringCopy(argsvAux2[2], BUFFER_MAX_LENGTH, argv[2]);
  argsvAux2[3] = NULL;

  char** argsvAux3 = malloc(sizeof(char*) * 4);
  argsvAux3[0] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  argsvAux3[1] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  argsvAux3[2] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  stringCopy(argsvAux3[0], BUFFER_MAX_LENGTH, argv[0]);
  stringCopy(argsvAux3[1], BUFFER_MAX_LENGTH, "-1");
  stringCopy(argsvAux3[2], BUFFER_MAX_LENGTH, argv[2]);
  argsvAux3[3] = NULL;

  char** argsvAux4 = malloc(sizeof(char*) * 4);
  argsvAux4[0] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  argsvAux4[1] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  argsvAux4[2] = (char* ) malloc(sizeof(char) * BUFFER_MAX_LENGTH);
  stringCopy(argsvAux4[0], BUFFER_MAX_LENGTH, argv[0]);
  stringCopy(argsvAux4[1], BUFFER_MAX_LENGTH, "-1");
  stringCopy(argsvAux4[2], BUFFER_MAX_LENGTH, argv[2]);
  argsvAux4[3] = NULL;
  

  global = 0;
  
  if (_sysFork() == 0) {
    _sysExecve(my_process_inc, 3, argsvAux);
  }
          
  if (_sysFork() == 0) {
    _sysExecve(my_process_inc, 3, argsvAux2);
  }

  if (_sysFork() == 0) {
    _sysExecve(my_process_inc, 3, argsvAux3);
  }
          
  if (_sysFork() == 0) {
    _sysExecve(my_process_inc, 3, argsvAux4);
  }
        

  _wait();

  printf("Final value: %d\n", global);

  return 0;
}