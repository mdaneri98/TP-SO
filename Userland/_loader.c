// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
/* _loader.c */
#include <stdint.h>
#include <string.h>
#include <syscalls.h>
#include <stdio.h>
#include <sh.h>

extern char bss;
extern char endOfBinary;


static void idle();
static void init();

typedef int (*processFunc)(int, char **);

void _start(processFunc process, int argc, char *argv[]) {
	if(process != NULL && (uint64_t) process != 1){
		process(argc, argv);
	} else if(process == NULL){
		init();
	} else{
		idle();
	}
	
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
