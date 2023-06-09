// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

#include <sh.h>
#include <constants.h>
#include <stdio.h>
#include <string.h>


int help(int argsc, char* argsv[]) {
    char *helpStr = "Predefined terminal programs:";
    printf("%s\n", helpStr);
    
    char **commandsName = getCommandsNames();
    char **commandsDesc = getCommandsDesc();
    for (int i = 0; i < CMDS_COUNT; i++) {
        printf("%d.%s: %s\n", i+1 , commandsName[i], commandsDesc[i]);
    }
    return 0;
}
