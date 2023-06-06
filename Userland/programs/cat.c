#include <stdio.h>
#include <string.h>
#include <cat.h>

#define BUFFER_MAX_LENGTH 1024
char c;

int cat(int argsc, char* argsv[]) {
    
    if (argsc <= 1) {
        char c;
        while(((c = getChar()) != -1)){
            putChar(c);
        }
    } else {
        for(int i=1; i<argsc ;i++){
            printString(argsv[i]);
            putChar(' ');
        }
        putChar('\n');
    }
    return 0;
}
