#include <stdio.h>
#include <string.h>


#define COUNT_BUFF_SIZE 12
char i;
int count;
char countString[COUNT_BUFF_SIZE];


int wc(int argsc, char* argsv[]) {
    //Recives from stdin or buffer
    if (argsc <= 1) {
        count = 1;  /* Starts in 1 line. */
        while((i = getChar()) != '&'){
            if(i == '\n'){
                count++;
            }
        }
        numToString(count, countString, COUNT_BUFF_SIZE);
        printf(countString);
    }
    
    return 0;
}
