#include <time.h>
#include <video.h>
#include <interrupts.h>

static uint64_t ticks = 0;
volatile uint64_t countDown = 0;
static uint8_t needsRestart = 0;

// static void shellRestart(){
// 	needsRestart = 0;
// 	console();
// }

void timer_handler() {
	if(countDown > 0)
		countDown--;
	// if(needsRestart > 0)
	// 	shellRestart();
	// ticks++;
}

int ticks_elapsed() {
	return ticks;
}

int seconds_elapsed() {
	return ticks / 18;
}

void sleep(uint64_t millis){
	countDown = millis / 55;
	while(countDown > 0)
		_hlt();
	return;
}

int aboutToRestart(){
	return needsRestart > 0;
}

// void restart(){
// 	needsRestart = 1;
// }