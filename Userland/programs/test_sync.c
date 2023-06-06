#include <stdint.h>
#include <stdio.h>
#include <syscalls.h>
#include <test_util.h>

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
  uint64_t pids[2 * TOTAL_PAIR_PROCESSES];

  if (argc != 3)
    return -1;

  char *argvDec[] = {argv[1], "-1", argv[2], NULL};
  char *argvInc[] = {argv[1], "1", argv[2], NULL};

  global = 0;

    uint64_t i, j;
    for (i = 0, j = TOTAL_PAIR_PROCESSES; i < TOTAL_PAIR_PROCESSES; i++, j++) {
        int indx = _sysFork();
        if (indx == 0) {
            _sysExecve(my_process_inc, 3, argvDec);
        }

        if (indx != 0) {
            pids[i] = indx;
        }
    
    
        int jndx = _sysFork();
        if (jndx != 0) {
            pids[j] = jndx;
        }

        if (jndx == 0) {
            _sysExecve(my_process_inc, 3, argvInc);
        }
    
        //pids[i + TOTAL_PAIR_PROCESSES] = my_create_process(my_process_inc, 3, argvInc);
    }


   _wait();
/*
  for (i = 0; i < TOTAL_PAIR_PROCESSES; i++) {
    my_wait(pids[i]);
    my_wait(pids[i + TOTAL_PAIR_PROCESSES]);
  }
*/

  printf("Final value: %d\n", global);

  return 0;
}