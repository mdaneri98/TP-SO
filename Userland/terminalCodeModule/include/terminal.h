#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdio.h>
// #include <string.h>
// #include <drawings.h>

#define PROMPT "user$ "
#define CMDS_COUNT 17
#define MAX_ARGS_COUNT 5

#define MINUTES 2
#define HOURS 4
#define DAY 7
#define MONTH 8
#define YEAR 9

void startTerminal();
void sh(int argc, char* argsv[]);

#endif
