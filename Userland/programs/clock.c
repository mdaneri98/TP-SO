#include <stdio.h>
#include <syscalls.h>

static int getFormat(int num);

int clock(int argsc, char *argsv[]){
    int hours;
    int minutes;
    int seconds;
    _clock(&hours, &minutes, &seconds);
    hours = getFormat(hours);
    minutes = getFormat(minutes);
    seconds = getFormat(seconds);

    printf("The system hour is %d:%d:%d\n", hours, minutes, seconds);
    
    return 0;
}

static int getFormat(int num){
	int dec = num & 240;
	dec = dec >> 4;
	int units = num & 15;
	return dec * 10 + units;
}