#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdio.h>
// #include <string.h>
// #include <drawings.h>

#define PROMPT "user$ "
#define CMDS_COUNT 17
#define MAX_ARGS_COUNT 5

#define BUFFER_MAX_LENGTH 250
#define CMDS_COUNT 17
#define MAX_ARGS_COUNT 5
#define MAX_LINES 50

#define TRUE 1
#define FALSE 0

#define MINUTES 2
#define HOURS 4
#define DAY 7
#define MONTH 8
#define YEAR 9

void startTerminal();
void sh(int argc, char* argsv[]);

char **getShLines();
unsigned int *getShLinecount();
char **getCommandsNames();
char **getCommandsDesc();
void clearLine(char *line);

#endif
