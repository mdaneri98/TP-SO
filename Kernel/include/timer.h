#ifndef _TIME_H_
#define _TIME_H_

#include <stdint.h>

uint64_t getTimerSize();
void sleep(uint64_t millis);
void updateTimers(uint64_t currentMillis);

#endif
