#ifndef _LIB_ASM_H
#define _LIB_ASM_H
void _setPIT();

void _setFrequency(uint8_t frequency);

uint8_t _getSound();

void _playSound(uint8_t tmp);

void intReset();

#endif // _LIB_ASM_H