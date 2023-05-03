#include <time.h>
#include <video.h>
#include <interrupts.h>

static uint64_t ticks = 0;
volatile uint64_t countDown = 0;

void timer_handler() {
	if(countDown > 0)
		countDown--;
	ticks++;
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