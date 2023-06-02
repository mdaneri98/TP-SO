#include <sh.h>


static void help() {
    char *helpStr = "Predefined terminal programs:";
    printf("%s\n", helpStr);
    char **lines = getShLines();
    unsigned int *lineCount = getShLinecount();
    char **commandsName = getCommandsNames();
    char **commandsDesc = getCommandsDesc();
    stringFormat(lines[(*lineCount)++ % MAX_LINES], BUFFER_MAX_LENGTH, "%s", helpStr);
    for (int i = 0; i < CMDS_COUNT; i++) {
        clearLine(lines[(*lineCount) % MAX_LINES]);
        stringFormat(lines[(*lineCount)++ % MAX_LINES], BUFFER_MAX_LENGTH, "%d.%s: %s", i+1 , commandsName[i], commandsDesc[i]);
        printf("%d.%s: %s\n", i+1 , commandsName[i], commandsDesc[i]);
    }
    
}
