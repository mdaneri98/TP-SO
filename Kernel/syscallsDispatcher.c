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
#include <pipe.h>
#include <bufferManagement.h>
#include <ps.h>
#include <sync.h>


#define TOTAL_SYSCALLS 26
#define AUX_BUFF_DIM 512

#define ERROR -1
#define NULL (void *)0

#define DEFAULT_FREQUENCY 1500

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
static uint64_t arqSysPs(uint64_t processes, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);
static uint64_t arqSysPriority(uint64_t pid, uint64_t newPriority, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);
static uint64_t arqSysChangeState(uint64_t pid, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);

static uint64_t arqSysSemOpen(uint64_t sem_id, uint64_t initialValue, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);
static uint64_t arqSysSemPost(uint64_t sem_id, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);
static uint64_t arqSysSemWait(uint64_t sem_id, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);
static uint64_t arqSysSemClose(uint64_t sem_id, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);

static uint64_t arqSysIdle(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);
static uint64_t arqSysWait(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);
static uint64_t arqSysExit(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7);

typedef uint64_t (*SyscallVec)(uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t r10, uint64_t r8, uint64_t r9, uint64_t rsp);

// SYSCALLS ARRAY
static SyscallVec syscalls[TOTAL_SYSCALLS];

IPCBuffer *stdinP;
IPCBuffer *stdoutP;
IPCBuffer *stderrP;

// EVERY NEW SYSCALL MUST BE LOADED IN THIS ARRAY
void setSyscalls(){
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
    syscalls[20] = (SyscallVec) arqSysIdle;
    syscalls[21] = (SyscallVec) arqSysPs;
    syscalls[22] = (SyscallVec) arqSysPriority;
    syscalls[23] = (SyscallVec) arqSysChangeState;
    syscalls[24] = (SyscallVec) arqSysWait;
    syscalls[25] = (SyscallVec) arqSysExit;

    /* Semaphores */
    syscalls[26] = (SyscallVec) arqSysSemOpen;
    syscalls[27] = (SyscallVec) arqSysSemPost;
    syscalls[28] = (SyscallVec) arqSysSemWait;
    syscalls[29] = (SyscallVec) arqSysSemClose;


    /* Pipes */
    stdinP = getSTDIN();
    stdoutP = getSTDOUT();
    stderrP = getSTDERR();

    for(int i=0; i<512; i++){
        stdinP->buffer[i] = '\0';
        stdoutP->buffer[i] = '\0';
        stderrP->buffer[i] = '\0';
    }
    for(int i=0; i<PD_SIZE ;i++){
        stdinP->references[i] = NULL;
        stdoutP->references[i] = NULL;
        stderrP->references[i] = NULL;
    }
    // We set the "pd" to the STDIN-STDOUT-STDERR entries
    stdinP->status = READ;
    stdinP->bufferDim = 0;
    stdinP->buffId = STDIN;
    stdinP->buffStart = 0;
    stdinP->opositeEnd = NULL;

    stdoutP->status = WRITE;
    stdoutP->bufferDim = 0;
    stdoutP->buffId = STDOUT;
    stdoutP->buffStart = 0;
    stdoutP->opositeEnd = NULL;
    
    stderrP->status = WRITE;
    stderrP->bufferDim = 0;
    stderrP->buffId = STDERR;
    stderrP->buffStart = 0;
    stderrP->opositeEnd = NULL;   
}

uint64_t syscallsDispatcher(uint64_t rax, uint64_t rdi, uint64_t rsi, uint64_t rdx, uint64_t rcx, uint64_t r8, uint64_t r9, uint64_t rsp) {    
    uint64_t toReturn = syscalls[rax](rdi, rsi, rdx, rcx, r8, r9, rsp);
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
static uint64_t arqSysRead(uint64_t pd, uint64_t buff, uint64_t count, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4){
    ProcessControlBlockADT current = getCurrentProcessEntry();
    if(current == NULL){
        return 0;
    }
    IPCBuffer *buffToRead = getPDEntry(current, pd);
    if(buffToRead == NULL || buffToRead->status == CLOSED || buffToRead->status == WRITE){
        return 0;
    }
    while(buffToRead->bufferDim == 0 && buffToRead->status != CLOSED){
        setProcessState(current, BLOCKED);
        int20h();
    }
    if(buffToRead->status == CLOSED){
        return 0;
    }
    char *auxBuff = (char *) buff;
    //FIXME: readBuffer general 
    if(buffToRead->buffId == PIPE){
        return readPipe(buffToRead,count, auxBuff);
    } else {
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
}

static uint64_t arqSysWrite(uint64_t pd, uint64_t buff, uint64_t count, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4) {
    ProcessControlBlockADT current = getCurrentProcessEntry();
    if(current == NULL){
        return 0;
    }
    //Pendant: find a way to send the correct pd when writing on a pipe 
    IPCBuffer *buffToWrite = getPDEntry(current, pd);
    if(buffToWrite == NULL || buffToWrite->status == CLOSED || buffToWrite->status == READ){
        return 0;
    }
    while(buffToWrite->bufferDim == PD_BUFF_SIZE && buffToWrite->status != CLOSED){
        setProcessState(current, BLOCKED);
        int20h();
    }
    if(buffToWrite->status == CLOSED){
        return 0;
    }

    char *auxBuff = (char *) buff;
    int bytesWritten = writeOnBuffer(buffToWrite, auxBuff, count);
    if(bytesWritten == 0){
        return 0;
    }
    if(buffToWrite->buffId == STDOUT){
        for(int i=0; i<bytesWritten ;i++){
            scrPrintChar(buffToWrite->buffer[i]);
            buffToWrite->buffer[i] = '\0';
            //buffToWrite->buffStart = (buffToWrite->buffStart+1)%PD_BUFF_SIZE;
            buffToWrite->bufferDim--;
        }
    } else if(buffToWrite->buffId == STDERR){
        Color red = { 0x0 , 0x0, 0xFF };
        for(int i=0; i<bytesWritten ;i++){
            scrPrintCharWithColor(buffToWrite->buffer[i], red);
            buffToWrite->buffer[i] = '\0';
            //buffToWrite->buffStart = (buffToWrite->buffStart+1)%PD_BUFF_SIZE;
            buffToWrite->bufferDim--;
        }
    } else if(buffToWrite->buffId == PIPE){
        actualizeReadBuff(buffToWrite);
    }
    
    return bytesWritten;
    /* OLD
   
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
    */
}

static uint64_t arqSysSemOpen(uint64_t sem_id, uint64_t initialValue, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    return semOpen((char*) sem_id, initialValue);
}

static uint64_t arqSysSemWait(uint64_t sem_id, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    return semWait((char*) sem_id);
}

static uint64_t arqSysSemPost(uint64_t sem_id, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    return semPost((char*) sem_id);
}

static uint64_t arqSysSemClose(uint64_t sem_id, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    return semClose((char*) sem_id);
}

static uint64_t arqSysBlock(uint64_t pid, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    ProcessControlBlockADT current = getEntry(pid);
    if(current == NULL){
        return -1;
    }
    setProcessState(current, BLOCKED);
    int20h();
    return 0;
}

static uint64_t arqSysUnblock(uint64_t pid, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    ProcessControlBlockADT current = getEntry(pid);
    if(current == NULL){
        return -1;
    }
    setProcessState(current, READY);
    int20h();
    return 0;
}

static uint64_t arqSysPriority(uint64_t pid, uint64_t newPriority, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7) {
    /* Proceso init y hlt no deben ser cambiados de prioridad. */
    if (pid == 1 || pid == 2) {
        return -1;
    }
    return changePriority(pid, newPriority);
}

static uint64_t arqSysChangeState(uint64_t pid, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7) {
    return changeState(pid);
}

// Lo modifiqué por que no debería matar el proceso actual, si no el proceso con el pid dado.
static uint64_t arqSysKill(uint64_t pid, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    if (pid < 3) {
        return -1;
    }
    ProcessControlBlockADT toKill = getEntry(pid);
    if(setProcessState(toKill, EXITED)){
        int20h();
        return 1;
    }
    return -1;
}

static uint64_t arqSysPs(uint64_t processes, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6) {
    int c = sysPs((ProcessData*) processes);
    return c;
}

static uint64_t arqSysExecve(uint64_t processFunction, uint64_t argc, uint64_t argv, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t rsp) {    
    return sysExecve((processFunc)processFunction, argc, (char**)argv, (void*)rsp);
}

static uint64_t arqSysFork(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t rsp) {
    return sysFork((void *)rsp);
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

static uint64_t arqSysIdle(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7){
    _hlt();
    return 0;
}

static uint64_t arqSysWait(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7){
    ProcessControlBlockADT current = getCurrentProcessEntry();
    while(hasOpenChilds(current)){
        setProcessState(current, BLOCKED);
        int20h();
    }
    return 0;
}

static uint64_t arqSysExit(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6, uint64_t nil7){
    setProcessState(getCurrentProcessEntry(), EXITED);
    int20h();
}
