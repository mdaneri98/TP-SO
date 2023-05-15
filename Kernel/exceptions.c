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
	scr_clear();
	scrChangeFont(1);
	routines[exception]();
	scr_printNewline();
	scr_printNewline();
	if(exception == 14){
		scrPrint("Error code: ");
		scr_printBase(errorCode, 10);
		scrPrint("\n\n");
	}
	scrPrint("Register dump:\n");
	scrPrint("RAX = ");
	scr_printHex(regdump[0]);
	scrPrint("\t\tRBX = ");
	scr_printHex(regdump[1]);
	scrPrint("\t\tRCX = ");
	scr_printHex(regdump[2]);
	scr_printChar('\n');
	scrPrint("RDX = ");
	scr_printHex(regdump[3]);
	scrPrint("\t\tRDI = ");
	scr_printHex(regdump[4]);
	scrPrint("\t\tRSI = ");
	scr_printHex(regdump[5]);
	scr_printChar('\n');
	scrPrint("RBP = ");
	scr_printHex(regdump[6]);
	scrPrint("\t\tRSP = ");
	scr_printHex(regdump[7]);
	scrPrint("\t\tR8 = ");
	scr_printHex(regdump[8]);
	scr_printChar('\n');
	scrPrint("R9 = ");
	scr_printHex(regdump[9]);
	scrPrint("\t\tR10 = ");
	scr_printHex(regdump[10]);
	scrPrint("\t\tR11 = ");
	scr_printHex(regdump[11]);
	scr_printChar('\n');
	scrPrint("R12 = ");
	scr_printHex(regdump[12]);
	scrPrint("\t\tR13 = ");
	scr_printHex(regdump[13]);
	scrPrint("\t\tR14 = ");
	scr_printHex(regdump[14]);
	scr_printChar('\n');
	scrPrint("R15 = ");
	scr_printHex(regdump[15]);
	scr_printChar('\n');
	scrPrint("RFLAGS = ");
	scr_printBase(regdump[16], 2);
	scrPrint("\t\tRIP = ");
	scr_printHex(regdump[17]);
	scr_printChar('\n');
	scr_printNewline();
	scrPrint("Press any key to continue");
	while(readBuffer() < 0) _hlt();
	scr_clear();

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