#include <speaker.h>
#include <stdint.h>
#include <time.h>
#include <libasm.h>
#include <video.h>


//Spreaker information found at: https://wiki.osdev.org/PC_Speaker

void speaker(uint64_t millis, uint64_t frequency);

static void playSound(uint64_t frequency);

static void noSound();

static void playSound(uint64_t frequency){
    uint64_t div;
    uint8_t tmp;

    div = 1193180 / frequency;               // Averiguar q es este numero
    _setPIT();
    _setFrequency((uint8_t)(div));
    _setFrequency((uint8_t)(div >> 8));

    tmp = _getSound();
  	if (tmp != (tmp | 3)) {
 		_playSound(tmp);
 	}
}

static void noSound(){
    uint8_t tmp = _getSound() & 0xFC;

    _playSound(tmp);
}

void speaker(uint64_t millis, uint64_t frequency){
    playSound(frequency);
    sleep(millis);
    noSound();
}