#ifndef INTERRUPS_H_
#define INTERRUPS_H_

#include <idtLoader.h>

void _irq00Handler(void);
void _irq01Handler(void);

void _exception0Handler(void);
void _exception5Handler(void);
void _exception6Handler(void);
void _exception14Handler(void);

void _syscallsHandler(void);

void _cli(void);

void _sti(void);

void _hlt(void);

void picMasterMask(uint8_t mask);

void picSlaveMask(uint8_t mask);
void startSystem();

//Termina la ejecución de la cpu.
void haltcpu(void);
#endif /* INTERRUPS_H_ */
