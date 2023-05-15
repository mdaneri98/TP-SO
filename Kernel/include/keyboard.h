#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

void addKey(uint8_t alKey);     // IRQ-0 HANDLER

char gameRead();
void resetBuffer();
char readBuffer();
char getScanCode(char c);
int toBuff(char c, unsigned char k);

#endif
