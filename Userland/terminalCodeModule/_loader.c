/* _loader.c */
#include <stdint.h>
#include <terminal.h>
#include <string.h>
#include <syscalls.h>
#include <stdio.h>

extern char bss;
extern char endOfBinary;

void * memset(void * destiny, int32_t c, uint64_t length);

typedef int (*processFunc)(int, char **);

void _start(processFunc process, int argc, char *argv[]) {
	if(process != NULL && (int) process != 1){
		int returnValue = process(argc, argv);
	} else if(process == NULL){
		init();
	} else{
		waiter();
	}
	// We need to implement the exit syscall
	// exit(return);
}

void init(){
	startTerminal();
}
void waiter(){
	while(1){
		_wait();
	}
}

void * memset(void * destiation, int32_t c, uint64_t length) {
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;

	while(length--)
		dst[length] = chr;

	return destiation;
}