#ifndef _LIB_ASM_H
#define _LIB_ASM_H

void _setPIT();
void _setFrequency(uint8_t frequency);
uint8_t _getSound();
void _playSound(uint8_t tmp);

int _hours();
int _seconds();
int _minutes();
uint16_t getCPUSpeed();
uint64_t readTimeStampCounter();

#endif // _LIB_ASM_H