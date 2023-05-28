
#include <stdio.h>
#include <string.h>
#include <syscalls.h>
#include <terminal.h>
#include <tron.h>
#include <lib.h>

#define BUFFER_MAX_LENGTH 250
#define CMDS_COUNT 12
#define MAX_ARGS_COUNT 5
#define PROMPT "user$ "
#define MAX_LINES 50
#define TRUE 1
#define FALSE 0

// Structures for help command
static char *commandsName[CMDS_COUNT];
static char *commandsDesc[CMDS_COUNT];
static void (*commandsFunction[CMDS_COUNT])(void);

// Structure for the command reader
static char lastCommand[BUFFER_MAX_LENGTH];
static unsigned int bufferCount = 0;

char lastArguments[MAX_ARGS_COUNT][100];  // Variable global para almacenar los argumentos
int cantidad_argumentos = 0;           // Variable global para almacenar la cantidad de argumentos

// Structure for maintaining commands history
static char lines[MAX_LINES][BUFFER_MAX_LENGTH] = {{0}};
static unsigned int lineCount = 0;

// Global variable for fontSize in this program
static int fontSize = 1;

// Prototypes
static void runProgram(int idx);
static int awaitCommand();
static int checkCommand(char* lastCommand);
static void sleep();

static void clear();
static void beep();
static void clock();
static void loadCommands();
static void printRegisters();
static void increment();
static void decrement();
static void playTron();
static void divZero();
static void invalidOpCode();
static void memoryDump();
static void refresh();
static void clearLines();
static void clearLine(char *line);
static int getArguments(char* lastCommand);
static void pageFault();

static void ps(int argsc, char* argsv[]);
static void nice(int argsc, char* argsv[]);

// Source code begins here
void sh(int argsc, char* argsv[]) {
    loadCommands();
    clearLines();

    ps(0, NULL);

    while(1) {
        printString(PROMPT);
        int idx = awaitCommand();
        if (idx >= 0 && idx < CMDS_COUNT) {
            runProgram(idx);
        } else {
            char *errMsg = "Invalid command. Use \'help\' to display the available commands with a description for their use";
            printString(errMsg);
            putChar('\n');
            
            clearLine(lines[lineCount % MAX_LINES]);
            stringCopy(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, errMsg);
        }

    }

}

static int awaitCommand() {
    int c;
    clearLine(lastCommand);
    while ((c = getChar()) != '\n') { 
        if (c != -1) {
            if(c == '\b' && bufferCount > 0){
                lastCommand[bufferCount--] = '\0';
                putChar(c);
            }
            else if(c != '\b'){
                // Add spaces instead, tab in video mode doesn't work very well yet
                if(c == '\t'){
                    c = ' ';
                    lastCommand[bufferCount++] = ' ';
                    putChar(c);
                    lastCommand[bufferCount++] = ' ';
                    putChar(c);
                    lastCommand[bufferCount++] = ' ';
                } else {
                    lastCommand[bufferCount++] = c;
                }
                putChar(c);
            }
        }
    }
    putChar('\n');
    lastCommand[bufferCount] = '\0';   // String finished, now we need to process it

    clearLine(lines[lineCount % MAX_LINES]);
    stringCopy(lines[lineCount % MAX_LINES], BUFFER_MAX_LENGTH, PROMPT);
    stringCopy(lines[lineCount++ % MAX_LINES] + stringLength(PROMPT), BUFFER_MAX_LENGTH - stringLength(PROMPT), lastCommand);

    int idx = checkCommand(lastCommand);
    
    bufferCount = 0;

    return idx;
}

static int checkCommand(char* lastCommand) {
    for (int i = 0; i < CMDS_COUNT; i++) {
        if (stringnCompare(commandsName[i], lastCommand, stringLength(commandsName[i])) == 0 
            && (lastCommand[stringLength(commandsName[i])] == ' ' || lastCommand[stringLength(commandsName[i])] == '\0')){
            return i;
        }
    }
    return -1;
}

static int getArguments(char *cadena) {
    int cantidad_argumentos = 0;
    
    // Eliminamos los espacios en blanco al comienzo y final de la cadena
    while (*cadena && (*cadena == ' ')) {
        cadena++;
    }

    // Si la cadena está vacía, no hay argumentos
    if (!*cadena) {
        return cantidad_argumentos;
    }

    // Utilizamos la función strtok() para separar la cadena en argumentos
    char *token = strtok(cadena, " ");

    // Guardamos los argumentos en la variable global
    while (token && cantidad_argumentos < MAX_ARGS_COUNT) {
        stringCopy(lastArguments[cantidad_argumentos], BUFFER_MAX_LENGTH, token);
        cantidad_argumentos++;
        token = strtok(NULL, " ");
    }

    return cantidad_argumentos;
}

static void runProgram(int idx) {
    if (idx >= 0 && idx < CMDS_COUNT) {
        _sysFork();
        int argsc = getArguments(lastCommand);
        _sysExecve(commandsFunction[idx], argsc, lastArguments);
    }
}

/* New functions */
static void ps(int argsc, char* argsv[]) {
    ProcessData data[256];

    int c = _sysPs(&data);
    for (int i = 0; i < c; i++)
        printf("ID: %d, Priority: %d, Stack: %x, Base: %x, Foreground: %d\n", data[i].id, data[i].priority, data[i].stack, data[i].baseStack, data[i].foreground);
}

static void nice(int argsc, char* argsv[]) {
    if (argsc < 2) {
        //Error.
    }
    _sysPriority(stringToInt(argsv[0]), stringToInt(argsv[1]));
}

static void block(int argsc, char* argsv[]) {
    _sysChangeState(stringToInt(argsv[0]));
}


/* Old functions */

static void help() {
    char *helpStr = "Predefined terminal programs:";
    printf("%s\n", helpStr);
    stringFormat(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, "%s", helpStr);
    for (int i = 0; i < CMDS_COUNT; i++) {
        clearLine(lines[lineCount % MAX_LINES]);
        stringFormat(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, "%d.%s: %s", i+1 , commandsName[i], commandsDesc[i]);
        printf("%d.%s: %s\n", i+1 , commandsName[i], commandsDesc[i]);
    }    
}

static void clear(){
    clearLines();
	_clear();
}

static void beep(){
    char *beepMsg= "I'm supposed to be beeping...";
    printf("%s\n", beepMsg);
    clearLine(lines[lineCount % MAX_LINES]);
    stringFormat(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, "%s", beepMsg);
	_beep(2000);
}

static int getFormat(int num) {
	int dec = num & 240;
	dec = dec >> 4;
	int units = num & 15;
	return dec * 10 + units;
}

static void clock() {
    int hours;
    int minutes;
    int seconds;
    _clock(&hours, &minutes, &seconds);
    hours = getFormat(hours);
    minutes = getFormat(minutes);
    seconds = getFormat(seconds);

    printf("The system hour is %d:%d:%d\n", hours, minutes, seconds);

    clearLine(lines[lineCount % MAX_LINES]);
    stringFormat(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, "The system hour is %d:%d:%d", hours, minutes, seconds);
}

static void loadCommands() {
    commandsName[0] = "clock";
    commandsDesc[0] = "Prints current time.";
    commandsFunction[0] = clock;
    commandsName[1] = "clear";
    commandsDesc[1] = "Clears the screen.";
    commandsFunction[1] = clear;
    commandsName[2] = "sleep";
    commandsDesc[2] = "Sleeps the machine.";
    commandsFunction[2] = sleep;
    commandsName[3] = "beep";
    commandsDesc[3] = "Makes a beep sound.";
    commandsFunction[3] = beep;
    commandsName[4] = "increment";
    commandsDesc[4] = "Increments font size.";
    commandsFunction[4] = increment;
    commandsName[5] = "decrement";
    commandsDesc[5] = "Decrements font size.";
    commandsFunction[5] = decrement;
    commandsName[6] = "tron";
    commandsDesc[6] = "Executes tron game.";
    commandsFunction[6] = playTron;
    commandsName[7] = "inforeg";
    commandsDesc[7] = "Prints the registers and their state when executed.";
    commandsFunction[7] = printRegisters;
    commandsName[8] = "memorydump";
    commandsDesc[8] = "Prints the 32 memory bytes next to a given hex direction.";
    commandsFunction[8] = memoryDump;
    commandsName[9] = "zeroexception";
    commandsDesc[9] = "Causes a Zero-Division exception. WARNING: this will restart the shell.";
    commandsFunction[9] = divZero;
    commandsName[10] = "invalidopcodeexception";
    commandsDesc[10] = "Causes an Invalid-OpCode exception. WARNING: this will restart the shell.";
    commandsFunction[10] = invalidOpCode;
    // commandsName[11] = "pagefaultexception";
    // commandsDesc[11] = "Causes an Page Fault exception. WARNING: this will restart the shell.";
    // commandsFunction[11] = pageFault;
    commandsName[11] = "help";
    commandsDesc[11] = "Prints the help manual for using the terminal.";
    commandsFunction[11] = help;
}

static void clearLine(char *line){
    for(int i=0; i<BUFFER_MAX_LENGTH ;line[i++] = 0);
}

static void clearLines(){
    for(int i=0; i<MAX_LINES ;i++)
        for(int j=0; j<BUFFER_MAX_LENGTH ;lines[i][j++] = 0)
    lineCount = 0;
}

static void refresh(){
    _clear();
    int aux;
    
    if(lineCount < MAX_LINES) 
        aux = 0;
    else
        aux = lineCount + 1;   // It's a circular array, I can round it until I load all the commands I need
    
    while((aux % MAX_LINES) != (lineCount % MAX_LINES)){
        if(lines[aux % MAX_LINES][0] != '\0'){
            printf("%s\n",lines[aux % MAX_LINES]);
        }
        aux++;
    }
}

static void printRegisters() {
    char * regsName[17] = {"RIP", "RAX", "RBX", "RCX", "RDX", "RDI", "RSI", "RBP", "RSP", "R8 ", "R9", "R10", "R11", "R12", "R13", "R14", "R15"};

    uint64_t regsInfo[17]; 
    int res = _getRegs(regsInfo);
    if(res == FALSE){
        char *regsErr = "Didn't make a register screenshot, press l-alt to make a capture of the register values";
        printf("%s\n", regsErr);
        clearLine(lines[lineCount % MAX_LINES]);
        stringFormat(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, "%s", regsInfo);
    }
    
    for(int i=0; i < 17; i++) {
        clearLine(lines[lineCount % MAX_LINES]);
        printf("%s = %x\n", regsName[i], regsInfo[i]);
        stringFormat(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, "%s = %x", regsName[i], regsInfo[i]);
    }

}

static void increment(){
    if(fontSize < 5){
        _changeFont(++fontSize);
        refresh();
    }
    else{
        char *incErr = "Reached max size of font.";
        printString(incErr);
        clearLine(lines[lineCount % MAX_LINES]);
        stringCopy(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, incErr);
        putChar('\n');
    }
}

static void decrement(){
    if(fontSize > 1){
        _changeFont(--fontSize);
        refresh();
    }
    else{
        char *decErr = "Reached min size of font.";
        printString(decErr);
        clearLine(lines[lineCount % MAX_LINES]);
        stringCopy(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, decErr);
        putChar('\n');
    }
}

static void playTron(){
    tron();
    _changeFont(fontSize);
    refresh();
}

static void divZero(){
    printf("About to cause an exception...\n");
    _sleep(1500);
    runDivzero();
}

static void invalidOpCode(){
    printf("About to cause an exception...\n");
    _sleep(1500);
    runInvalidOpcode();
}

static void memoryDump(){
    uint8_t dump[32];
    int argsc = getArguments(lastCommand);
    unsigned long direction = 0;
    /*
    if(*argument == 0 || !isHex(argument)){
        char *argErr = "You need to specify a memory direction in hex format.";
        stringCopy(lines[lineCount++ % BUFFER_MAX_LENGTH], BUFFER_MAX_LENGTH, argErr);
        printString(argErr);
        putChar('\n');
        return;
    }
    else if(*argument == '0' && *(argument + 1) == 0)
        direction = 0;
    else{
        direction = stringHexToNum(argument);
    }
*/

    _memoryDump((uint64_t *)direction, dump);
    clearLine(lines[lineCount % MAX_LINES]);
    
    printf("%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\n", 
    dump[0], dump[1], dump[2], dump[3], dump[4], dump[5], dump[6], dump[7],
    dump[8], dump[9], dump[10], dump[11], dump[12], dump[13], dump[14], dump[15]);

    stringFormat(lines[lineCount % MAX_LINES], BUFFER_MAX_LENGTH, 
    "%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x",
    dump[0], dump[1], dump[2], dump[3], dump[4], dump[5], dump[6], dump[7],
    dump[8], dump[9], dump[10], dump[11], dump[12], dump[13], dump[14], dump[15]);

    clearLine(lines[++lineCount % MAX_LINES]);
    
    printf("%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\n", 
    dump[16], dump[17], dump[18], dump[19], dump[20], dump[21], dump[22], dump[23],
    dump[24], dump[25], dump[26], dump[27], dump[28], dump[29], dump[30], dump[31]);
    
    stringFormat(lines[lineCount % MAX_LINES], BUFFER_MAX_LENGTH, 
    "%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x",
    dump[16], dump[17], dump[18], dump[19], dump[20], dump[21], dump[22], dump[23],
    dump[24], dump[25], dump[26], dump[27], dump[28], dump[29], dump[30], dump[31]);


    lineCount++;
}

static void pageFault(){
    printf("About to cause an exception...");
    _sleep(1500);
    runPageFault();
}

static void sleep() {
    int c = getArguments(lastArguments);
    unsigned long millis = 0;
    if(*lastArguments == 0)
        millis = 2000;
    else if(*lastArguments == '0' && *(lastArguments + 1) == 0)
        millis = 0;
    else{
        millis = stringToNum(lastArguments);
        if(millis == 0){
            char *argErr = "You can enter only one number.";
            stringCopy(lines[lineCount++ % BUFFER_MAX_LENGTH], BUFFER_MAX_LENGTH, argErr);
            printString(argErr);
            putChar('\n');
            return;
        }
    }
    _sleep(millis);
}