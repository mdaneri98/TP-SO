#include <programs.h>
#include <video.h>
#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <keyboard.h>
#include <time.h>
#include <speaker.h>


static char * DEFINITIONS[COMMANDS] = 	{"Clock: Returns current time","Clear: Erases everything currently in the screen","Sleep: Pauses the system for a while",
										"Beep: Makes a sound","Help: Shows the current commands available and can expand on them"};
int getFormat(int num) {
	int dec = num & 240;
	dec = dec >> 4;
	int units = num & 15;
	return dec * 10 + units;
}

void clock() {
	int sec_BCD = _seconds();
	int min_BCD = _minutes();
	int hour_BCD = _hours();
	int seconds = getFormat(sec_BCD);
	int mins = getFormat(min_BCD);
	int hours = getFormat(hour_BCD);
	scr_printBase(hours,10);
	scr_print(":");
	scr_printBase(mins,10);
	scr_print(":");
	scr_printBase(seconds,10);
	scr_printNewline();
	return;
}

void clear(){
	scr_clear();
}

void sleepTerminal(){
	sleep(2000);
}

void help(){
	int line = getLength();
	if(line == 4){
	scr_print("Current commands that can be run in this terminal:");
	scr_printNewline();
	for(int i = 0;i < COMMANDS - 1;i++){
		scr_printBase(i,10);
		scr_print(") ");
		scr_print(CMDS[i]);
		scr_printNewline();
	}
	scr_print("To learn more use help <number of instruction to learn>");
	}else if(line == 6){
		long num = toNum(5);
		scr_print(DEFINITIONS[num]);
	}

	scr_printNewline();
}

void beep(){
	scr_print("Beeping...");
	speaker(2000, 8000);
	scr_printNewline();
}