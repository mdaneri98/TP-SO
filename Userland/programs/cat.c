#include <stdio.h>
#include <string.h>
#include <cat.h> //actualizar include path

#define BUFFER_MAX_LENGTH 1024
char c;

int cat(int argsc, char* argsv[]) {
    
    if (argsc <= 1) {
        int i;
        //while hasta que presionen ctrl+c
        while(1){
            i = 0;
            while(((c = getChar()) != -1) && i < BUFFER_MAX_LENGTH){
                putChar(c);
            }
        }
    } else {
        printString(argsv[1]);
    }
    return 0;
}
