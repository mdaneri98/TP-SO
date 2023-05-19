#include <video.h>
#include <time.h>
#include <idtLoader.h>
#include <keyboard.h>
#include <interrupts.h>

#define REGDUMP_SIZE 18

extern void restartmain(void);

typedef void (*exceptionRoutines)(void);

static void zero_division();

static void invalid_opcode();

static void bounds_exception();

static void page_fault();

static exceptionRoutines routines[15];

void set_EXCEPTIONS(){
	routines[0] = zero_division;
	routines[5] = bounds_exception;
	routines[6] = invalid_opcode;
	routines[14] = page_fault;
}

void exceptionDispatcher(int exception, uint64_t regdump[REGDUMP_SIZE], uint64_t errorCode) {
	scrClear();
	scrChangeFont(1);
	routines[exception]();
	scrPrintNewline();
	scrPrintNewline();
	if(exception == 14){
		scrPrint("Error code: ");
		scrPrintBase(errorCode, 10);
		scrPrint("\n\n");
	}
	scrPrint("Register dump:\n");
	scrPrint("RAX = ");
	scrPrintHex(regdump[0]);
	scrPrint("\t\tRBX = ");
	scrPrintHex(regdump[1]);
	scrPrint("\t\tRCX = ");
	scrPrintHex(regdump[2]);
	scrPrintChar('\n');
	scrPrint("RDX = ");
	scrPrintHex(regdump[3]);
	scrPrint("\t\tRDI = ");
	scrPrintHex(regdump[4]);
	scrPrint("\t\tRSI = ");
	scrPrintHex(regdump[5]);
	scrPrintChar('\n');
	scrPrint("RBP = ");
	scrPrintHex(regdump[6]);
	scrPrint("\t\tRSP = ");
	scrPrintHex(regdump[7]);
	scrPrint("\t\tR8 = ");
	scrPrintHex(regdump[8]);
	scrPrintChar('\n');
	scrPrint("R9 = ");
	scrPrintHex(regdump[9]);
	scrPrint("\t\tR10 = ");
	scrPrintHex(regdump[10]);
	scrPrint("\t\tR11 = ");
	scrPrintHex(regdump[11]);
	scrPrintChar('\n');
	scrPrint("R12 = ");
	scrPrintHex(regdump[12]);
	scrPrint("\t\tR13 = ");
	scrPrintHex(regdump[13]);
	scrPrint("\t\tR14 = ");
	scrPrintHex(regdump[14]);
	scrPrintChar('\n');
	scrPrint("R15 = ");
	scrPrintHex(regdump[15]);
	scrPrintChar('\n');
	scrPrint("RFLAGS = ");
	scrPrintBase(regdump[16], 2);
	scrPrint("\t\tRIP = ");
	scrPrintHex(regdump[17]);
	scrPrintChar('\n');
	scrPrintNewline();
	scrPrint("Press any key to continue");
	while(readBuffer() < 0) _hlt();
	scrClear();

	return;
}

static void zero_division() {
	scrPrint("Zero Division Exception\n");
}
static void invalid_opcode(){
	scrPrint("Invalid Opcode Exception\n");
}

static void page_fault(){
	scrPrint("Page fault exception\n");
}

static void bounds_exception(){
	scrPrint("Page fault exception\n");
}