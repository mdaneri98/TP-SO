#include <stdio.h>
#include <string.h> 
#include <filter.h>

#define BUFFER_MAX_LENGTH 1024
#define NUMBER_OF_VOCALS 10

char filterBuff[BUFFER_MAX_LENGTH];
char t;

void cleanBuffer(char * buffer);
int isVocal(char c);
char vocals[] = {'A', 'a', 'E', 'e', 'I', 'i', 'O', 'o', 'U', 'u'};
void stringFilterCopy(char * arg);

int filter(int argsc, char* argsv[]) {
    //recibe por stdin o por un buffer
    if (argsc <= 1) {
        while(((t = getChar()) != -1)){
            if(!isVocal(t)){
                putChar(t);
            }
        }
    } else {
        for(int i=1; i<argsc ;i++){
            stringFilterCopy(argsv[i]);
            printString(filterBuff);
            cleanBuffer(filterBuff);
            putChar(' ');
        }
        putChar('\n');
    }
    return 0;
}


void stringFilterCopy(char * arg){
    int sourceIndex = 0;
    int destIndex = 0;
    while((arg[sourceIndex] != '\0') && destIndex < BUFFER_MAX_LENGTH){
        if(!isVocal(arg[sourceIndex])){
            filterBuff[destIndex] = arg[sourceIndex];
            destIndex++;
        }
        sourceIndex++;
    }
    return;
}

int isVocal(char c){
    int i;
    for(i = 0; i < NUMBER_OF_VOCALS; i++){
        if(c == vocals[i]){
            return 1;
        }
    }
    return 0;
}

void cleanBuffer(char * buffer){
    int i = 0;
    while((buffer[i] != '\0')&& i < BUFFER_MAX_LENGTH){
        buffer[i] = '\0';
        i++;
    }
    return;
}

