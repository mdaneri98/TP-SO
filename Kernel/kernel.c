// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <lib.h>
#include <moduleLoader.h>
#include <idtLoader.h>
#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <syscallsDispatcher.h>

#define USERLAND_STACK 0x800000

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static const int lock;

static void *sampleCodeModuleAddress = (void*)0x400000;
static void *sampleDataModuleAddress = (void*)0x500000;

typedef int (*EntryPoint)();


void clearBSS(void * bssAddress, uint64_t bssSize){
	memset(bssAddress, 0, bssSize);
}

void * getStackBase(){
	return (void*)(
		(uint64_t)&endOfKernel
		+ PageSize * 8				//The size of the stack itself, 32KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * getUserSpaceStackBase(){
	return (void*)(
		USERLAND_STACK
		+ PageSize * 64				//The size of the stack itself, 64KiB
		- sizeof(uint64_t)			//Begin at the top of the stack
	);
}

void * initializeKernelBinary(){
	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	clearBSS(&bss, &endOfKernel - &bss);
	return getStackBase();
}

int main(){	
	createMemoryManager((void*) USERLAND_STACK, (uint64_t) getUserSpaceStackBase() - USERLAND_STACK);

	setSyscalls();

	createInit();

	loadIDT();

	_startSystem();

	return 0;
}
