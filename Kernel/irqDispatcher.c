#include <time.h>
#include <keyboard.h>
#include <stdint.h>
#include <intervalTimer.h>

static void int_20();
static void int_21();

typedef void (*irqFunctions)();


// VECTOR PARA LAS IRQ NO CAMBIAR XFA ES LA TERCERA VEZ QUE LO PONGO AAAAAA
static irqFunctions functions[2];

// SI AGREGAN UNA IRQ NUEVA LA METEN EN ESTE VECTOR Y AUMENTAN SU TAMAÑO
void set_IRQ(){
	functions[0] = int_20;
	functions[1] = int_21;
}

void irqDispatcher(uint64_t irq) {
	functions[irq]();
}

void int_20() {
	timer_handler();
}

void int_21() {
	keyboard_handler();
}