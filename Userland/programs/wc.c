#include <stdio.h>
#include <string.h>
#include <cat.h> //actualizar include path

#define COUNT_BUFF_SIZE 12
char i;
int count;
char countString[COUNT_BUFF_SIZE];


int wc(int argsc, char* argsv[]) {
    //recibe por stdin o por un buffer
    if (argsc <= 1) {
        count = 0;
        while((i = getChar()) != -1){
            if(i == '\n'){
                count++;
            }
        }
        numToString(count, countString, COUNT_BUFF_SIZE);
        printString(countString);
    } 
    /*else {
        
    }*/
    return 0;
}
