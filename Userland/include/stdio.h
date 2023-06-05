#ifndef _STDIO_H_
#define _STDIO_H_

#include <stdarg.h>

#define NULL (void *) 0

int printf(const char *str, ...);

void scanf(const char *str, ...);

int printString(char* buffer);

char getChar();

int putChar(char c);

void stringFormat(char *buffer, unsigned int bufferDim, const char *str, ...);

#endif