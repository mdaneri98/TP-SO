#ifndef LIB_H
#define LIB_H

#include <stdio.h>
#include <stdint.h>

#define TRUE 1
#define FALSE 0

int stringToInt(const char *str);

void *malloc(uint64_t size);
void free(void *ptr);
void *realloc(void *ptr, uint64_t size);



#endif