#ifndef INTERRUPS_H_
#define INTERRUPS_H_

void _irq00Handler();
void _irq01Handler();

void _exception0Handler();
void _exception5Handler();
void _exception6Handler();
void _exception14Handler();

void _syscallsHandler();

void _cli();
void _sti();
void _hlt();

void _startSystem();

// Wrapper of timer tick interrupt (called via software)
void _int20h();

#endif /* INTERRUPS_H_ */
