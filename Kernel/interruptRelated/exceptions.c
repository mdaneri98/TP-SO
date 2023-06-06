#include <video.h>
#include <time.h>
#include <idtLoader.h>
#include <keyboard.h>
#include <interrupts.h>
#include <scheduler.h>
#include <constants.h>

#define REGDUMP_SIZE 18

static void zeroDivision();
static void invalidOpcode();
static void boundsException();
static void pageFault();
static void notHandledException();

void exceptionDispatcher(int exception, uint64_t regdump[REGDUMP_SIZE], uint64_t errorCode) {
	scrClear();
	scrChangeFont(1);

	switch(exception){
		case 0: zeroDivision(); break;
		case 5: boundsException(); break;
		case 6: invalidOpcode(); break;
		case 14: pageFault(); break;
		default: notHandledException(); break;
	}
	
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
	char c;
	// We wait until the user presses any key to kill this process
	syscallsDispatcher(0, STDIN, &c, 1, (uint64_t) NULL, (uint64_t) NULL, (uint64_t) NULL,(uint64_t) NULL);
	scrClear();
	setProcessState(getCurrentProcessEntry(), EXITED);
    _int20h();
}

static void zeroDivision() {
	scrPrint("Zero Division Exception\n");
}
static void invalidOpcode(){
	scrPrint("Invalid Opcode Exception\n");
}

static void pageFault(){
	scrPrint("Page fault Exception\n");
}

static void boundsException(){
	scrPrint("Index out of Bounds Exception\n");
}

static void notHandledException(){
	scrPrint("Unknown Exception\n");
}