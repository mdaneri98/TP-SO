/* _loader.c */
#include <stdint.h>
#include <string.h>
#include <syscalls.h>
#include <stdio.h>
#include <sh.h>

extern char bss;
extern char endOfBinary;

void * memset(void * destiny, int32_t c, uint64_t length);
static void idle();
static void init();

typedef int (*processFunc)(int, char **);

void _start(processFunc process, int argc, char *argv[]) {
	if(process != NULL && (int) process != 1){
		int returnValue = process(argc, argv);
	} else if(process == NULL){
		init();
	} else{
		idle();
	}
	// We need to implement the exit syscall
	_sysExit();
}

static void init(){
	sh(0, NULL);
}
static void idle(){
	while(1){
		_idle();
	}
}

void * memset(void * destiation, int32_t c, uint64_t length) {
	uint8_t chr = (uint8_t)c;
	char * dst = (char*)destiation;

	while(length--)
		dst[length] = chr;

	return destiation;
}