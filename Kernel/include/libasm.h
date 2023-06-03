#ifndef _LIB_ASM_H
#define _LIB_ASM_H

#include <stdint.h>

void _setPIT();
void _setFrequency(uint8_t frequency);
uint8_t _getSound();
void _playSound(uint8_t tmp);

int _hours();
int _seconds();
int _minutes();

void _picMasterMask(uint8_t mask);
void _picSlaveMask(uint8_t mask);

uint64_t _getCPUCristalSpeed();
uint64_t _getTSCNumerator();
uint64_t _getTSCDenominator();
uint64_t _readTimeStampCounter();

#endif // _LIB_ASM_H