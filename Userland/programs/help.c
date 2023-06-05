
#include <sh.h>
#include <stdio.h>
#include <string.h>

#define BUFFER_MAX_LENGTH 250
#define MAX_ARGS_COUNT 25
#define CMDS_COUNT 21


int help(int argsc, char* argsv[]) {
    char *helpStr = "Predefined terminal programs:";
    printf("%s\n", helpStr);
    
    char **commandsName = getCommandsNames();
    char **commandsDesc = getCommandsDesc();
    for (int i = 0; i < CMDS_COUNT; i++) {
        printf("%d.%s: %s\n", i+1 , commandsName[i], commandsDesc[i]);
    }
    
}
