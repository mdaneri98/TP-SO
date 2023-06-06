#include <stdio.h>
#include <syscalls.h>
#include <test_util.h>

#define MINOR_WAIT 1000000  // TODO: Change this value to prevent a process from flooding the screen
#define WAIT 100000000      // TODO: Change this value to make the wait long enough to see theese processes beeing run at least twice

#define TOTAL_PROCESSES 6
#define LOWEST 0      // TODO: Change as required
#define MEDIUM_LOW 1  // TODO: Change as required
#define MEDIUM 2      // TODO: Change as required
#define MEDIUM_HIGH 3 // TODO: Change as required
#define HIGH 4        // TODO: Change as required
#define SUPER 5       // TODO: Change as required

int64_t prio[TOTAL_PROCESSES] = {LOWEST, MEDIUM_LOW, MEDIUM, MEDIUM_HIGH, HIGH, SUPER};

int test_prio(int argsc, char* argsv[]) {
  int64_t pids[TOTAL_PROCESSES];
  char *argv[] = {0};
  uint64_t i;

  for (i = 0; i < TOTAL_PROCESSES; i++){
    int j = _sysFork();
    if (j != 0)
      pids[i] = j;
    
    if(j == 0){
        _sysExecve(endless_loop_print, 0, argv);
    }
  }

  bussy_wait(WAIT);
  printf("\nCHANGING PRIORITIES...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++){
    _sysPriority(pids[i], prio[i]);   
  }

  bussy_wait(WAIT);
  printf("\nBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _sysBlock(pids[i]);

  printf("CHANGING PRIORITIES WHILE BLOCKED...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _sysPriority(pids[i], MEDIUM);

  printf("UNBLOCKING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _sysUnblock(pids[i]);

  bussy_wait(WAIT);
  printf("\nKILLING...\n");

  for (i = 0; i < TOTAL_PROCESSES; i++)
    _sysKill(pids[i]);


    return 0;
}
