#include <stdint.h>
#include <video.h>

/* Los prototipos de los syscalls internos deben estar en el .c así no se tiene acceso desde afuera. */
// #include <kernelSyscalls.h>

#include <keyboard.h>
#include <string.h>
#include <time.h>
#include <speaker.h>
#include <libasm.h>
#include <memory.h>
#include <process.h>
#include <scheduler.h>
#include <interrupts.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2
#define TOTAL_SYSCALLS 21
#define AUX_BUFF_DIM 512

#define ERROR -1
#define NULL (void *)0

#define DEFAULT_FREQUENCY 1500

buffer_t stdin;
buffer_t stdout;
buffer_t stderr;

static uint64_t arqSysRead(uint64_t buff, uint64_t dim, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5);

static uint64_t arqSysWrite(uint64_t pd, uint64_t buff, uint64_t count, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4);
static uint64_t arqSysMemoryDump(uint64_t direction, uint64_t buffer, uint64_t nil1, uint64_t nil2, uint64_t  nil3, uint64_t nil4, uint64_t nil5);
static uint64_t arqSysGetRegistersInfo(uint64_t buffer, uint64_t nil1, uint64_t  nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysClear(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);
static uint64_t arqSysBeep(uint64_t millis, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysSleep(uint64_t millis, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysClock(uint64_t hours, uint64_t minutes, uint64_t seconds, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4);
static uint64_t arqSysScreenHeight(uint64_t height, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysScreenWidth(uint64_t width, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysGameRead(uint64_t data, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysgetPtrToPixel(uint64_t x, uint64_t y, uint64_t color, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4);
static uint64_t arqSysDrawLine(uint64_t fromX, uint16_t fromY, uint16_t toX, uint16_t toY, uint64_t color, uint64_t nil1, uint64_t nil2);
static uint64_t arqSysGetPenX(uint64_t x, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysGetPenY(uint64_t y, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysChangeFontSize(uint64_t newSize, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);

static uint64_t arqSysBlock(uint64_t pid, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysKill(uint64_t pid, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);
static uint64_t arqSysExecve(uint64_t processFunction, uint64_t argc, uint64_t argv, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t rsp);
static uint64_t arqSysFork(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);

static uint64_t arqSysWait(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);

typedef uint64_t (*SyscallVec)(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9, uint64_t rsp);

// SYSCALLS ARRAY
static SyscallVec syscalls[TOTAL_SYSCALLS];


// EVERY NEW SYSCALL MUST BE LOADED IN THIS ARRAY
void set_SYSCALLS(){
    syscalls[0] = (SyscallVec) arqSysRead;
    syscalls[1] = (SyscallVec) arqSysWrite;
    syscalls[2] = (SyscallVec) arqSysClear;
    syscalls[3] = (SyscallVec) arqSysBeep;
    syscalls[4] = (SyscallVec) arqSysSleep;
    syscalls[5] = (SyscallVec) arqSysClock;
    syscalls[6] = (SyscallVec) arqSysScreenHeight;
    syscalls[7] = (SyscallVec) arqSysScreenWidth;
    syscalls[8] = (SyscallVec) arqSysgetPtrToPixel;
    syscalls[9] = (SyscallVec) arqSysGameRead;
    syscalls[10] = (SyscallVec) arqSysDrawLine;
    syscalls[11] = (SyscallVec) arqSysGetPenX;
    syscalls[12] = (SyscallVec) arqSysGetPenY;
    syscalls[13] = (SyscallVec) arqSysChangeFontSize;
    syscalls[14] = (SyscallVec) arqSysMemoryDump;
    syscalls[15] = (SyscallVec) arqSysGetRegistersInfo;

    syscalls[16] = (SyscallVec) arqSysBlock;
    syscalls[17] = (SyscallVec) arqSysKill;
    syscalls[18] = (SyscallVec) arqSysExecve;
    syscalls[19] = (SyscallVec) arqSysFork;
    syscalls[20] = (SyscallVec) arqSysWait;
    for(int i=0; i<512; i++){
        stdin.buffer[i] = '\0';
        stdout.buffer[i] = '\0';
        stderr.buffer[i] = '\0';
    }
    for(int i=0; i<PD_SIZE ;i++){
        stdin.references[i] = NULL;
        stdout.references[i] = NULL;
        stderr.references[i] = NULL;
    }
    // We set the "pd" to the STDIN-STDOUT-STDERR entries
    stdin.status = READ;
    stdin.bufferDim = 0;
    stdin.buffId = STDIN;

    stdout.status = WRITE;
    stdout.bufferDim = 0;
    stdout.buffId = STDOUT;

    stderr.status = WRITE;
    stderr.bufferDim = 0;
    stderr.buffId = STDERR;
}

uint64_t syscallsDispatcher(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9, uint64_t rsp) {    
    uint64_t toReturn = syscalls[rax](rdi, rsi, rdx, r10, r8, r9, rsp);
    return toReturn;
}

/**
 * @brief Reads the given buffer and drops it's content into the file descriptor given, and only the amount specified
 * 
 * @param pd
 * @param buff 
 * @param count 
 * @return Amount of bytes read. -1 in case of error
 */
static uint64_t arqSysRead(uint64_t pd, uint64_t buff, uint64_t count, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5){
    PCBNode *current = getCurrentProcess();
    if(current == NULL){
        return 0;
    }
    buffer_t *buffToRead = current->pcbEntry.pdTable[pd];
    if(buffToRead == NULL || buffToRead->status == CLOSED || buffToRead->status == WRITE){
        return 0;
    }
    while(buffToRead->bufferDim == 0 && buffToRead->status != CLOSED){
        current->pcbEntry.state = BLOCKED;
        int20h();
    }
    if(buffToRead->status == CLOSED){
        return 0;
    }
    char *auxBuff = (char *) buff;
    int bytesRead = 0;
    char c;
    while( bytesRead < count && buffToRead->bufferDim > 0){
        c = buffToRead->buffer[bytesRead];
        buffToRead->bufferDim--;
        auxBuff[bytesRead++] = c;
    }
    for(int j=bytesRead, k=0; j<PD_BUFF_SIZE; j++, k++){
        buffToRead->buffer[k] = buffToRead->buffer[j];
    }
    return bytesRead;
}

static uint64_t arqSysWrite(uint64_t pd, uint64_t buff, uint64_t count, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4) {
    PCBNode *current = getCurrentProcess();
    if(current == NULL){
        return 0;
    }
    buffer_t *buffToWrite = current->pcbEntry.pdTable[pd];
    if(buffToWrite == NULL || buffToWrite->status == CLOSED || buffToWrite->status == READ){
        return 0;
    }
    while(buffToWrite->bufferDim == PD_BUFF_SIZE && buffToWrite->status != CLOSED){
        current->pcbEntry.state = BLOCKED;
        int20h();
    }
    if(buffToWrite->status == CLOSED){
        return 0;
    }
    char *auxBuff = (char *) buff;
    int bytesWritten = 0;
    char c;
    while( bytesWritten < count && buffToWrite->bufferDim < PD_BUFF_SIZE - 1){
        c = auxBuff[bytesWritten++];
        buffToWrite->buffer[buffToWrite->bufferDim++] = c;
    }
    if(buffToWrite->buffId == STDOUT){
        for(int i=0; i<bytesWritten ;i++){
            scrPrintChar(buffToWrite->buffer[i]);
            buffToWrite->buffer[i] = '\0';
            buffToWrite->bufferDim--;
        }
    }
    else if(buffToWrite->buffId == STDERR){
        Color red = { 0x0 , 0x0, 0xFF };
        for(int i=0; i<bytesWritten ;i++){
            scrPrintCharWithColor(buffToWrite->buffer[i], red);
            buffToWrite->buffer[i] = '\0';
            buffToWrite->bufferDim--;
        }
    }
    return bytesWritten;
}

static uint64_t arqSysBlock(uint64_t pid, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    sysBlock(pid);
    return 0;
}

static uint64_t arqSysKill(uint64_t pid, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    sysKill(pid);
    return 0;
}

static uint64_t arqSysExecve(uint64_t processFunction, uint64_t argc, uint64_t argv, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t rsp) {
    /* FIXME: Error en la conversión de argv.
    processFunc pFunc = (processFunc) processFunction;
    
    char* argvAux[] = (char **) argv;
    return sysExecve(pFunc, argc, argvAux, rsp);
    */
   return 0;
}

static uint64_t arqSysFork(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7) {
    return sysFork();
}

static uint64_t arqSysMemoryDump(uint64_t direction, uint64_t buffer, uint64_t nil1, uint64_t nil2, uint64_t  nil3, uint64_t nil4, uint64_t nil5) {
    printMemory((uint64_t *)direction, (uint8_t *)buffer);
    return 0;
}

static uint64_t arqSysGetRegistersInfo(uint64_t buffer, uint64_t nil1, uint64_t  nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    return getRegistersInfo((uint64_t *) buffer);
}

// Clear
static uint64_t arqSysClear(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7){
    resetBuffer();
    scrClear();
    return 0;
}

static uint64_t arqSysBeep(uint64_t millis, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    speaker(millis, DEFAULT_FREQUENCY);
    return 0;
}

static uint64_t arqSysSleep(uint64_t millis, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    sleep(millis);
    return 0;
}

static uint64_t arqSysClock(uint64_t hours, uint64_t minutes, uint64_t seconds, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4){
    int *auxHours = (int *) hours;
    int *auxMinutes = (int *) minutes;
    int *auxSeconds = (int *) seconds;
    *auxHours = _hours();
    *auxMinutes = _minutes();
    *auxSeconds = _seconds();
    return 0;
}

static uint64_t arqSysScreenHeight(uint64_t height, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    uint16_t *h = (uint16_t *) height;
    *h = scrGetHeight();
    return 0;
}

static uint64_t arqSysScreenWidth(uint64_t width, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    uint16_t *w = (uint16_t *) width;
    *w = scrGetWidth();
    return 0;
}

static uint64_t arqSysGameRead(uint64_t data, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    unsigned char *d = (unsigned char *) data;
    *d = gameRead();
    return 0;
}

static uint64_t arqSysgetPtrToPixel(uint64_t x, uint64_t y, uint64_t color, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4){
    Color *c = (Color *) color;
    Color *screenPixel = (Color *) getPtrToPixel(x,y);
    c->b = screenPixel->b;
    c->r = screenPixel->r;
    c->g = screenPixel->g;
    return 0;
}

static uint64_t arqSysDrawLine(uint64_t fromX, uint16_t fromY, uint16_t toX, uint16_t toY, uint64_t color, uint64_t nil6, uint64_t nil7){
    Color *auxColor = (Color *) color;
    Color auxColor2;
    auxColor2.b = auxColor->b;
    auxColor2.g = auxColor->g;
    auxColor2.r = auxColor->r;
    scrDrawLine((uint16_t)fromX, (uint16_t)fromY, (uint16_t)toX, (uint16_t)toY, auxColor2);
    return 0;
}

static uint64_t arqSysGetPenX(uint64_t x, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    uint16_t *xAux = (uint16_t *) xAux;
    *xAux = scrGetPenX();
    return 0;
}


static uint64_t arqSysGetPenY(uint64_t y, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    uint16_t *yAux = (uint16_t *) yAux;
    *yAux = scrGetPenY();
    return 0;
}

static uint64_t arqSysChangeFontSize(uint64_t newSize, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    if(newSize > 0 && newSize < 6)
        scrChangeFont(newSize);
    return 0;
}

static uint64_t arqSysWait(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7){
    _hlt();
    return 0;
}

buffer_t *getSTDIN(){
    return &stdin;
}
buffer_t *getSTDOUT(){
    return &stdout;
}
buffer_t *getSTDERR(){
    return &stderr;
}