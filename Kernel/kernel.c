#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <libasm.h>
#include <moduleLoader.h>
#include <video.h>
#include <idtLoader.h>
#include <time.h>
#include <scheduler.h>
#include <interrupts.h>
#include <memoryManager.h>
#include <exceptions.h>
#include <syscallDispatcher.h>

#define USERLAND_STACK 0x800000

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

void * getUserSpaceStackBase(){
	return (void*)(
		USERLAND_STACK
		+ PageSize * 32				//The size of the stack itself, 32KiB
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
	createMemoryManager(USERLAND_STACK, getUserSpaceStackBase() - USERLAND_STACK);

	set_SYSCALLS();
  	set_EXCEPTIONS();

	createInit();

	load_idt();

	startSystem();

	return 0;
}
