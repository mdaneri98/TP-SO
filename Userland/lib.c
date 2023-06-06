// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com


#include <stdio.h>
#include <syscalls.h>
#include <stdlib.h>


void *malloc(uint64_t size) {
    return _sysMalloc(size);
}

void free(void *ptr) {
    _sysFree(ptr);
}

void *realloc(void *ptr, uint64_t size) {
    return _sysRealloc(ptr, size);
}


int stringToInt(const char *str) {
    int result = 0;
    int sign = 1;
    int i = 0;

    if (str[0] == '-') {
        sign = -1;
        i = 1;
    }

    // Iterave over chars.
    while (str[i] != '\0') {
        // Verify if char is valid.
        if (str[i] >= '0' && str[i] <= '9') {
            result = result * 10 + (str[i] - '0');
            i++;
        } else {
            break;
        }
    }

    // Apply sign
    result *= sign;

    return result;
}



void memset(void * destination, int32_t c, uint64_t length)
{
    uint8_t chr = (uint8_t)c;
    char * dst = (char*)destination;

    while(length--)
        dst[length] = chr;

    //return destination;
}