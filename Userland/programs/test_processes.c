// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <syscalls.h>
#include <test_util.h>

enum State { RUNNING,
             BLOCKED,
             KILLED };

typedef struct P_rq {
  int32_t pid;
  enum State state;
} p_rq;

int test_processes(int argc, char *argv[]) {
  uint8_t rq;
  uint8_t alive = 0;
  uint8_t action;
  uint64_t max_processes;
  char *argvAux[] = {0};

  // if (argc != 2)
  //   return -1;

  // if ((max_processes = satoi(argv[1])) <= 0)
  // return -1;

  max_processes = 5;

  p_rq p_rqs[max_processes];

  while (1) {
    
    // Create max_processes processes
    for (rq = 0; rq < max_processes; rq++) {
        int i = _sysFork();
        if (i == 0) {
            _sysExecve(endless_loop, 0, argvAux);
        }

        if (i != 0) {
            p_rqs[rq].pid = i;
            // _yield();
        }

      if (p_rqs[rq].pid == -1) {
        printf("test_processes: ERROR creating process\n");
        return -1;
      } else {
        p_rqs[rq].state = RUNNING;
        alive++;
      }
    }
    _yield();
    _setToBackground();
    // Randomly kills, blocks or unblocks processes until every one has been killed
    while (alive > 0) {

      for (rq = 0; rq < max_processes; rq++) {
        action = GetUniform(100) % 2;

        switch (action) {
          case 0:
            if (p_rqs[rq].state == RUNNING || p_rqs[rq].state == BLOCKED) {
              if (_sysKill(p_rqs[rq].pid) == -1) {
                printf("test_processes: ERROR killing process\n");
                return -1;
              }
              p_rqs[rq].state = KILLED;
              alive--;
            }
            break;

          case 1:
            if (p_rqs[rq].state == RUNNING) {
              if (_sysBlock(p_rqs[rq].pid) == -1) {
                printf("test_processes: ERROR blocking process\n");
                return -1;
              }
              p_rqs[rq].state = BLOCKED;
            }
            break;
        }
      }

      // Randomly unblocks processes
      for (rq = 0; rq < max_processes; rq++)
        if (p_rqs[rq].state == BLOCKED && GetUniform(100) % 2) {
          if (_sysUnblock(p_rqs[rq].pid) == -1) {
            printf("test_processes: ERROR unblocking process\n");
            return -1;
          }
          p_rqs[rq].state = RUNNING;
        }
        _setToForeground();
    }

  }
}