#include <stdio.h>
#include <lib.h>
#include <string.h>
#include <syscalls.h>
#include <cat.h> //actualizar include path

#define COUNT_BUFF_SIZE 12
char c;
int count;
char * countString[COUNT_BUFF_SIZE];


int wc(int argsc, char* argsv[]) {
    //recibe por stdin o por un buffer
    if (argsc <= 1) {
        count = 0;
        while((c = getChar()) != -1){
            if(c == '\n'){
                count++;
            }
        }
        putChar(numToString(count, countString, COUNT_BUFF_SIZE));
    } 
    /*else {
        
    }*/
    return 0;
}
