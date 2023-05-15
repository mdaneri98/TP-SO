/* sampleCodeModule.c */
#include<stdint.h>
#include"functions.h"

extern uint64_t write(const char * buf);
int main() {
	write("USER!!!!");
	return 0;
}