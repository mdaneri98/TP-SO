#include <stdint.h>
#include <string.h>
#include <lib.h>
#include <moduleLoader.h>
//#include <naiveConsole.h>
#include <video.h>
#include <programs.h>
#include <idtLoader.h>
#include <time.h>

extern uint8_t text;
extern uint8_t rodata;
extern uint8_t data;
extern uint8_t bss;
extern uint8_t endOfKernelBinary;
extern uint8_t endOfKernel;

static const uint64_t PageSize = 0x1000;

static const int LOCK;

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
	clearBSS(&bss, &endOfKernel - &bss);

	char buffer[10];
	scr_printNewline();
	scr_print("[x64BareBones]");
	scr_printNewline();
	scr_print("CPU Vendor: ");
	scr_print(cpuVendor(buffer));
	scr_printNewline();
	scr_print("[Loading modules]");
	scr_printNewline();

	void * moduleAddresses[] = {
		sampleCodeModuleAddress,
		sampleDataModuleAddress
	};

	loadModules(&endOfKernelBinary, moduleAddresses);
	scr_print("[Done]");
	scr_printNewline();
	scr_printNewline();
	scr_print(" [Initializing kernel's binary]");
	scr_printNewline();

	//clearBSS(&bss, &endOfKernel - &bss);

	scr_print(" text: 0x");
	scr_printHex((uint64_t)&text);
	scr_printNewline();
	scr_print(" rodata: 0x");
	scr_printHex((uint64_t)&rodata);
	scr_printNewline();
	scr_print(" data: 0x");
	scr_printHex((uint64_t)&data);
	scr_printNewline();
	scr_print(" bss: 0x");
	scr_printHex((uint64_t)&bss);
	scr_printNewline();
	scr_print("[Done]");
	scr_printNewline();
	scr_printNewline();
	return getStackBase();
}

int main()
{	
	load_idt();

	sleep(2000);

	scr_clear();

	scr_print("[Kernel Main]");
	scr_printNewline();
	scr_print(" Sample code module at 0x");
	scr_printHex((uint64_t)sampleCodeModuleAddress);
	scr_printNewline();
	scr_print(" Calling the sample code module returned: ");
	scr_printHex(((EntryPoint)sampleCodeModuleAddress)());
	scr_printNewline();
	scr_printNewline();

	scr_print("  Sample data module at 0x");
	scr_printHex((uint64_t)sampleDataModuleAddress);
	scr_printNewline();
	scr_print("  Sample data module contents: ");
	scr_print((char*)sampleDataModuleAddress);
	scr_printNewline();
	clock();
	scr_printNewline();

	scr_print("[Finished]");
	scr_printNewline();
	scr_printNewline();
	sleep(1500);
	
	//scr_clear();
	scr_print(PROMPT);
	
	while(1);

	return 0;
}
