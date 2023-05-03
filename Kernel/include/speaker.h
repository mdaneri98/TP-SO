#ifndef _SPEAKER_H_
#define _SPEAKER_H_

#include <stdint.h>

void speaker(uint64_t millis, uint64_t frequency);

static void playSound(uint64_t frequency);

static void noSound();

#endif // _SPEAKER_H_