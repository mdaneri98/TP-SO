#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
#include <video.h>
#include <idtLoader.h>
#include <time.h>
#include <scheduler.h>
#include <interrupts.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static const int lock;

static void * const sampleCodeModuleAddress = (void*)0x400000;
static void * const sampleDataModuleAddress = (void*)0x500000;

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize)
{
	memset(bssAddress, 0, bssSize);
}

void * getStackBase()
{
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary()
{
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);
	return getStackBase();
}

int main()
{	
	// Hay que inicializar el memory manager del userSpace antes de que siquiera entre en juego el 
	//timer tick, se puede dar la race condition de que ejecute el scheduler con memoria que no está inicializada
	createDefaultMemoryManager(((uint64_t)getStackBase() - PageSize), (uint64_t)&endOfKernel + PageSize * 7 - sizeof(uint64_t));
	// La función getStackBase obtiene la base del stack a partir de la finalización del binario del Kernel
	//se entra con este valor a la función Main del kernel, la idea sería dejarle una página al kernel para que tenga su
	//stack asegurado de que ningún otro proceso pueda pisarlo, pero no sé si eso es correcto hacer esto último, dejaría
	//disponible para el espacio de usuario un total de 7 páginas para que use, es decir, si usamos el tamaño por defecto
	//de nuestra memoria actual, en el momento en el que se hagan más de 7 peticiones de memoria en simultáneo, cagamos xd
	//hay que averiguar si hay alguna forma de expandir ese número, o reducir el tamaño del bloque de memoria que se reserva

	createInit();

	load_idt();
	startSystem();

	return 0;
}
