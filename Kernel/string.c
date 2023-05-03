#include <string.h>
#include <video.h>
#include <stdint.h>

#define DELTA 'a' - 'A'
#define NULL ((void * )0)

/*
stringCompare: Returns -1 if str1 is smaller than str2, 1 otherwise, and 0 if they are equal.
*/
int stringCompare(const char *str1, const char *str2){
    unsigned char c1, c2;
	while (1) {
		c1 = *str1++;
		c2 = *str2++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
	}
	return 0;
}

/*
stringnCompare: Compares the first n chars from str1 and str2 and returns -1 if str1 is smaller than
                str2, 1 otherwise, and 0 if they are equal
*/
int stringnCompare(const char *str1, const char *str2, uint64_t count){
	unsigned char c1, c2;
	while (count) {
		c1 = *str1++;
		c2 = *str2++;
		if (c1 != c2)
			return c1 < c2 ? -1 : 1;
		if (!c1)
			break;
		count--;
	}
	return 0;
}

/*
toUpper: Transforms all characters of str from lower case to upper case
*/
void toUpper(char *str){
    for(int i=0; str[i] != '\0' ;i++)
        if(str[i] >= 'a' && str[i] <= 'z')
            str[i] -= DELTA;
}

/*
toLower: Transforms all characters of str from upper case to lower case
*/
void toLower(char *str){
    for(int i=0; str[i] != '\0' ;i++)
        if(str[i] >= 'A' && str[i] <= 'Z')
            str[i] += DELTA;
}

/*
stringCopy: Copies content from src to dest and returns its location
*/
char *stringCopy(char *dest, int destSize,const char *source){
    char *temp = dest;
    while((*temp++ = *source++) != '\0' || destSize-- != 0);
    *temp = '\0';       // If destSize reached 0
    return dest;
}

/*
stringnCopy: Copies n chars from src to dest and returns its location
*/
char *stringnCopy(char *dest, int destSize, const char *src, int count){
    char *temp = dest;
    while(count || destSize){
        if((*temp = *src) != '\0')
            src++;
        temp++;
        count--;
        destSize--;
    }
    *temp = '\0';       // If destSize reached 0
    return dest;
}

/*
stringLength: Returns the length of the str (NULL included)
*/
uint64_t stringLength(const char *str){
    int length;
    for(length=0; str[length]!='\0' ;length++);
    return length;
}

/*
firstOcurrence: Returns the location of the first ocurrence of a character, or NULL if it's not found
*/
char *firstOcurrence(char *str, char c){
    char *tmp = str;
    for(int i=0; tmp[i] != '\0' ;i++)
        if(tmp[i] == c)
            return tmp;
    return NULL;
}

/*
stringToNum: Returns the number value of a string, or 0 if it's not a number
*/
long stringToNum(const char * str){
    double num = 0;
    double neg = 1;
    if(str[0] == '-'){
        neg*=-1.0;
        str++;
    }
    for(int i = 0; str[i] != 0; i++) {
        num *= 10;
        if(!isCharNum(str[i]))
            return 0;
        num += charToNum(str[i]); 
    }
    return num*neg;
}

/*
charToNum: Returns the number value of a character, or 0 if it's not a number
*/
int charToNum(char c){
    if(isCharNum(c))
        return c - '0';
    return 0;
}

/*
isCharNum: Returns if a character is a number's ASCII value
*/
int isCharNum(char c){
    return c >= '0' && c <= '9';
}


/*
numToString: Transforms a number to a string
*/
char *numToString(int num, char *buffer, int bufferSize){
    if(bufferSize <= 0) return 0;

    if(num == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return 1;
    }
    int isNeg = 0;
    if(num < 0) {
        buffer[0] = '-';
        num *= -1;
        isNeg = 1;
    }
    long aux = num;
    long digitcount = 0;
    while(aux > 0) {
        aux /= 10;
        digitcount++;
    }
    
    long startPos = digitcount + isNeg - 1;
    if(startPos + 1 > bufferSize) return 0;
    buffer[startPos+1] = '\0';
    for(int i = 0; i < digitcount; i++) {
        buffer[startPos - i] = num % 10 + '0';
        num /= 10;
    }
    return startPos + 1;
}