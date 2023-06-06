#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdio.h>


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
