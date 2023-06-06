
#include <stdio.h>
#include <constants.h>
#include <string.h>
#include <syscalls.h>
#include <tron.h>
#include <lib.h>
#include <sh.h>

#include <ps.h>
#include <kill.h>
#include <block.h>
#include <phylo.h>
#include <loop.h>
#include <nice.h>
#include <cat.h>
#include <filter.h>
#include <wc.h>
#include <clock.h>
#include <invalid_op_code.h>
#include <div_zero.h>
#include <sleep.h>
#include <memory_dump.h>
#include <page_fault.h>
#include <help.h>

#include <test_mm.h>
#include <test_prio.h>
#include <test_processes.h>
#include <test_sync.h>


// Structures for help command
static char *commandsName[CMDS_COUNT];
static char *commandsDesc[CMDS_COUNT];
static int (*commandsFunction[CMDS_COUNT])(int, char **);

// Structure for the command reader
static char lastCommand[BUFFER_MAX_LENGTH*2];
static unsigned int bufferCount = 0;
static char* secondCommand = NULL;  /* Apunta dos direcciones posteriores de donde se ubica el | en lastCommand, si así es el caso. */

char lastArguments[MAX_ARGS_COUNT][BUFFER_MAX_LENGTH];  // Variable global para almacenar los argumentos

// Structure for maintaining commands history
static char lines[MAX_LINES][BUFFER_MAX_LENGTH] = {{0}};
static unsigned int lineCount = 0;

// Global variable for fontSize in this program
static int fontSize = 1;

// Prototypes
static void runProgram(int idx, int jdx);
static void awaitCommand(int* idx, int* jdx);
static int checkCommand(char* lastCommand);

static int clear();
static int beep(int argsc, char* argsv[]);
static void loadCommands();
static int printRegisters(int argsc, char* argsv[]);
static int increment(int argsc, char* argsv[]);
static int decrement(int argsc, char* argsv[]);
static int playTron(int argsc, char* argsv[]);
static void refresh();
static void clearLines();
void clearLine(char *line);
static int getArguments(int idx, char* lastCommand);


// Source code begins here
void sh(int argsc, char* argsv[]) {
    loadCommands();
    clearLines();


    while(1) {
        printString(PROMPT);

        int idx;    /* Index in the commandsFunction of the first program */
        int jdx;    /* Index in the commandsFunction of the second program */
        awaitCommand(&idx, &jdx);

        if (idx >= 0 && idx < CMDS_COUNT) {            
            runProgram(idx, jdx);
        } else {
            char *errMsg = "Invalid command. Use \'help\' to display the available commands with a description for their use";
            printString(errMsg);
            putChar('\n');
            
            clearLine(lines[lineCount % MAX_LINES]);
            stringCopy(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, errMsg);
        }

    }

}

static void awaitCommand(int* idx, int* jdx) {
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

    *idx = checkCommand(lastCommand);
    
    /* Vemos el caso si el input fue pipeado */
    secondCommand = firstOcurrence(lastCommand, '|');
    if (secondCommand == NULL) {
        *jdx = -1;
    } else {
        secondCommand += 2; /* Luego del pipe, tiene que haber un espacio. */
        *jdx = checkCommand(secondCommand);
    }

    bufferCount = 0;
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

/* Almacena los argumentos en la variable lastArguments, dada la función obtenida por idx. */
static int getArguments(int idx, char *cadena) {
    int cantidad_argumentos = 0;
    /*
    stringCopy(lastArguments[cantidad_argumentos], BUFFER_MAX_LENGTH, commandsName[idx]);
    cantidad_argumentos++;
    */
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

static void runProgram(int idx, int jdx) {    
    
    int commandLength = stringLength(lastCommand);
    int background = lastCommand[commandLength-1] == '&';

    if (idx >= 0 && idx < CMDS_COUNT) {
        if (jdx >= 0 && jdx < CMDS_COUNT) { /* Caso si el input está pipeado. */
            int pipefd[2];
            _pipe(pipefd);

            int argsc1 = getArguments(idx, lastCommand);
            if (_sysFork() == 0) {
                char** argumentsAux = malloc(sizeof(char*) * MAX_ARGS_COUNT);
                for (int i = 0; i < argsc1; i++) {
                    argumentsAux[i] = malloc(sizeof(char) * BUFFER_MAX_LENGTH);
                    for (int j = 0; j < BUFFER_MAX_LENGTH; j++) {
                        argumentsAux[i][j] = lastArguments[i][j];
                    }
                }

                _close(1);
                _close(pipefd[0]);
                _dup2(1, pipefd[1]);

                _sysExecve(commandsFunction[idx], argsc1, (char**) argumentsAux);
            }
            _yield();
            int argsc2 = getArguments(jdx, secondCommand);
            if (_sysFork() == 0) {
                char** argumentsAux = malloc(sizeof(char*) * MAX_ARGS_COUNT);
                for (int i = 0; i < argsc2; i++) {
                    argumentsAux[i] = malloc(sizeof(char) * BUFFER_MAX_LENGTH);
                    for (int j = 0; j < BUFFER_MAX_LENGTH; j++) {
                        argumentsAux[i][j] = lastArguments[i][j];
                    }
                }

                _close(0);
                _close(pipefd[1]);
                _dup2(0, pipefd[0]);
                
                
                _sysExecve(commandsFunction[jdx], argsc2, (char**) argumentsAux);
            }
            _yield();
            _close(pipefd[0]);
            _close(pipefd[1]);
        } else {
            // El background solo funcionará para comandos sin pipe.
    
            if (_sysFork() == 0) {
                int argsc = getArguments(idx, lastCommand);

                char** lastArgumentsAux = malloc(sizeof(char*) * MAX_ARGS_COUNT);
                for (int i = 0; i < argsc; i++) {
                    lastArgumentsAux[i] = malloc(sizeof(char) * BUFFER_MAX_LENGTH);
                    for (int j = 0; j < BUFFER_MAX_LENGTH; j++) {
                        lastArgumentsAux[i][j] = lastArguments[i][j];
                    }
                }
                
                if (background != 0)
                    _setToBackground();
                
                _sysExecve(commandsFunction[idx], argsc, (char**) lastArgumentsAux);
            }
        }

        //El proceso padre sigue ejecutando normalmente
        if (!background)    
            _wait();
    
    }
}



/* Old functions */

static int clear(){
    clearLines();
	_clear();
    return 0;
}

static int beep(int argsc, char* argsv[]){
    char *beepMsg= "I'm supposed to be beeping...";
    printf("%s\n", beepMsg);
    clearLine(lines[lineCount % MAX_LINES]);
    stringFormat(lines[lineCount++ % MAX_LINES], BUFFER_MAX_LENGTH, "%s", beepMsg);
	_beep(2000);
    return 0;
}

/*
static int getFormat(int num) {
	int dec = num & 240;
	dec = dec >> 4;
	int units = num & 15;
	return dec * 10 + units;
}
*/

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
    
    /* Los programas nuevos no deberían ser programas built-in, por eso, no deberían estar en este arreglo.
    Por simplicidad, usamos el mismo vector. */
    commandsName[12] = "ps";
    commandsDesc[12] = "Prints important data for each process in the system.";
    commandsFunction[12] = ps;
    commandsName[13] = "kill";
    commandsDesc[13] = "Kills the process that matches with the id given.";
    commandsFunction[13] = kill;
    commandsName[14] = "block";
    commandsDesc[14] = "Blocks the process that matches with the id given.";
    commandsFunction[14] = kill;
    commandsName[15] = "nice";
    commandsDesc[15] = "Changes the priority of the process with the id given to the new priority.";
    commandsFunction[15] = nice;
    commandsName[16] = "phylo";
    commandsDesc[16] = "Starts the phylo process. You can add a filosopher with the a keyword and r to remove a filosopher.";
    commandsFunction[16] = phylo;
    commandsName[17] = "cat";
    commandsDesc[17] = "Copies input to output";
    commandsFunction[17] = cat;
    commandsName[18] = "filter";
    commandsDesc[18] = "Filters vocals from input";
    commandsFunction[18] = filter;
    commandsName[19] = "wc";
    commandsDesc[19] = "Counts input lines";
    commandsFunction[19] = wc;
    commandsName[20] = "loop";
    commandsDesc[20] = "Prints a message every constant time with the process id";
    commandsFunction[20] = loop;
    commandsName[21] = "test_sync";
    commandsDesc[21] = "Execute test of sync";
    commandsFunction[21] = NULL;
    commandsName[22] = "test_prio";
    commandsDesc[22] = "Execute test of priority";
    commandsFunction[22] = test_prio;
    commandsName[23] = "test_mm";
    commandsDesc[23] = "Execute test of memory manager";
    commandsFunction[23] = test_mm;
    commandsName[24] = "test_processes";
    commandsDesc[24] = "Execute test of processes";
    commandsFunction[24] = test_processes;
    commandsName[25] = "test_sync";
    commandsDesc[25] = "Execute test of sync";
    commandsFunction[25] = test_sync;

}

void clearLine(char *line){
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

static int printRegisters(int argsc, char* argsv[]) {
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
    return 0;
}

static int increment(int argsc, char* argsv[]){
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
    return 0;
}

static int decrement(int argsc, char* argsv[]){
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
    return 0;
}

static int playTron(int argsc, char* argsv[]){
    tron();
    _changeFont(fontSize);
    refresh();
    return 0;
}


char **getCommandsNames(){
    return commandsName;
}
char **getCommandsDesc(){
    return commandsDesc;
}