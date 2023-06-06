// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdint.h>
#include <video.h>
#include <keyboard.h>
#include <string.h>
#include <interrupts.h>
#include <bufferManagement.h>

#define CODES 56
#define LETTERS 25
#define PRINT 47

#define R_SHIFT 0x36
#define L_SHIFT 0x2A

#define C_CODE 0x2E
#define D_CODE 0x20

#define L_CNTRL 0x1D

#define BUFFER_DIM 512
#define ERROR -1
#define NULL (void *)0

static int isBuffEmpty();
static int isMapped(unsigned char key);
static void shift();
static void capsLock();
static int isArrow(int index);
static int isCapslock(int index);
static int isBuffEmpty();
static void cntrl();

static char kBuff[BUFFER_DIM] = {0};
static char sBuff[BUFFER_DIM] = {0};

static uint64_t writePosition = 0;
static uint64_t readPosition = 0;
static uint64_t keyCount = 0;

/*For special keys:
n-> enter
 -> space
b-> backspace
c-> capslock
t->tab
u->up arrow key
l->left arrow key
r->right arrow key
d->down arrow key
Scan and break codes found at: https://stanislavs.org/helppc/make_codes.html an on https://wiki.osdev.org/PS/2_Keyboard using table one
*/
const char KEYS[CODES][2] = {   {'a','A'},{'b','B'},{'c','C'},{'d','D'},{'e','E'},{'f','F'},{'g','G'},{'h','H'},{'i','I'},{'j','J'},{'k','K'},{'l','L'},{'m','M'},{'n','N'},{'o','O'},
                                {'p','P'},{'q','Q'},{'r','R'},{'s','S'},{'t','T'},{'u','U'},{'v','V'},{'w','W'},{'x','X'},{'y','Y'},{'z','Z'},
                                {'1','1'},{'2','2'},{'3','3'},{'4','4'},{'5','5'},{'6','6'},{'7','7'},{'8','8'},{'9','9'},{'0','0'},
                                {'-','-'},{'=','='},{'[','['},{']',']'},{';',';'},{'\'','\''},{'`','`'},{'\\','\\'},{',',','},{'.','.'},{'/','/'},
                                {' ',' '},{'\n','\n'},{'\b','\b'},{'c','c'},{'\t','\t'},{'u','u'},{'l','l'},{'r','r'},{'d','d'}};

const char SHIFTED[CODES][2] = {    {'A','a'},{'B','b'},{'C','c'},{'D','d'},{'E','e'},{'F','f'},{'G','g'},{'H','h'},{'I','i'},{'J','j'},{'K','k'},{'L','l'},{'M','m'},{'N','n'},{'O','o'},
                                    {'P','p'},{'Q','q'},{'R','r'},{'S','s'},{'T','t'},{'U','u'},{'V','v'},{'W','w'},{'X','x'},{'Y','y'},{'Z','z'},
                                    {'!','!'},{'@','@'},{'#','#'},{'$','$'},{'%','%'},{'^','^'},{'&','&'},{'*','*'},{'(','('},{')',')'},
                                    {'_','_'},{'+','+'},{'{','{'},{'}','}'},{':',':'},{'"','"'},{'~','~'},{'|','|'},{'<','<'},{'>','>'},{'?','?'},
                                    {' ',' '},{'\n','\n'},{'\b','\b'},{'c','c'},{'\t','\t'},{'u','u'},{'l','l'},{'r','r'},{'d','d'}};

const unsigned char SCAN_CODES[CODES] = {   0x1E,0x30,0x2E,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,0x31,0x18,
                                            0x19,0x10,0x13,0x1F,0x14,0x16,0x2F,0x11,0x2D,0x15,0x2C,
                                            0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,
                                            0x0C,0x0D,0x1A,0x1B,0x27,0x28,0x29,0x2B,0x33,0x34,0x35,
                                            0x39,0x1C,0x0E,0x3A,0x0F,0x48,0x4B,0x4D,0x50};


static int lock = 0;
static int shiftKey = 0;
static int leftCntrl = 0;
// static int rightCntrl = 0;
// static int lineLength = 0;
/*SCAN CODES
0x01    esc             0x02    1           0x03    2       0x04    3               0x05    4           0x06    5             (implementada)
0x07    6               0x08    7           0x09    8         0x0A    9               0x0B    0           0x0C    -           (implementada)
0x0D    =               0x0E    backspace     0x0F    tab     0x10    Q                 0x11    W           0x12    E           (implementada)
0x13    R               0x14    T             0x15    Y         0x16    U                 0x17    I           0x18    O           (implementada)
0x19    P                 0x1A    [             0x1B    ]       0x1C    enter             0x1D    left ctrl    0x1E    A             (left ctrl missing)
0x1F    S               0x20    D           0x21    F          0x22    G               0x23    H           0x24    J             (implementada)
0x25    K                 0x26    L             0x27     ;      0x28    '                 0x29    `             0x2A    left shift  (implemented)
0x2B    \               0x2C    Z             0x2D    X         0x2E    C                 0x2F    V           0x30    B           (implemented)
0x31    N                 0x32    M           0x33    ,       0x34    .                 0x35    /             0x36    right shift (implemented)
0x39    space             0x3A    Caps        0x48    UP      0x4B    Left            0x4D    Right       0x50    Down        (implemented)
Down: 0x50


THE FOLLOWING KEYS WILL NOT BE IMPLEMENTED
0x38    left alt        0x3B    F1          0x3C    F2        0x3D    F3                 0x3E    F4             0x3F    F5
0x40    F6                 0x41    F7          0x42    F8         0x43    F9              0x44    F10         0x57    F11
0x45    NumLock         0x46    ScrollLock

KEYPAD:
0x47     7              0x48     8             0x49    9         0x4A     -                 0x4B     4          0x4C     5
0x4D     6                 0x4E     +          0x4F    1       0x50     2                 0x51     3            0x52     0
0x53     .              0x37     *
*/

void addKey(uint8_t alKey) {
    unsigned char key = (unsigned char) alKey;
    char item;

    // Check whether a character is mapped or not
    if(key > 0x80){
        if((key == R_SHIFT + 0x80) || (key == L_SHIFT + 0x80)){
            shift();
        } else if(key == L_CNTRL + 0x80){
            cntrl();
        }
        return;

    } else if((key == R_SHIFT) || (key == L_SHIFT)){
        shift();
        return;
    } else if(key == L_CNTRL){
        cntrl();
        return;
    }
    int idx = isMapped(key);
    if(idx == -1){
        return;
    }
    if(leftCntrl && key == C_CODE){
        ProcessControlBlockADT foregroundProcess = getForegroundProcess();
        if(foregroundProcess != NULL){
            setProcessState(foregroundProcess, EXITED);
            _int20h();
        }
        return;
    } else if(leftCntrl && key == D_CODE){
        toBuff('\0', key);  // EOF
        return;
    }
    if(isCapslock(idx)){
        capsLock();
        return;
    }
    if(shiftKey){
        item = SHIFTED[idx][lock];
    } else{
        item = KEYS[idx][lock];
    }
    if(item == '\t' && shiftKey == 1){
        return;
    }
    if(isArrow(idx)){
        item = 0xFF;        // DEFAULT NOT MAPPED KEY VALUE
    }
    toBuff(item, key);
}

static int isMapped(unsigned char key){
    for(int i = 0; i < CODES;i++){
        if(SCAN_CODES[i] == key){
            return i;
        }
    }
    return ERROR;
}

static void cntrl(){
    leftCntrl = !leftCntrl;
}

static void shift(){
    shiftKey = !shiftKey;
}

static void capsLock(){
    if(lock == 0){
        lock = 1;
    } else{
        lock = 0;
    }
}

static int isArrow(int index){
    return index > 51;
}

static int isCapslock(int index){
    return index == 50;
}

int toBuff(char c, unsigned char k){
    IPCBufferADT stdin = getSTDIN();
    uint32_t bufferDim = getBufferDim(stdin);
    copyToBuffer(stdin, &c, 1);
    if(bufferDim == 0){
        setBufferReferencesReady(stdin);
    }
    return 1;
}

char readKeyboardBuffer(){
    if (isBuffEmpty())
    {
        return -1;
    }
    keyCount--;
    char c = kBuff[readPosition % BUFFER_DIM];
    readPosition++;
    return c;
}

char gameRead(){
    if (isBuffEmpty())
    {
        return -1;
    }
    keyCount--;
    char c =  sBuff[readPosition % BUFFER_DIM];
    readPosition++;
    return c;
}

static int isBuffEmpty(){
    return keyCount < 1;
}

void resetBuffer(){
    for(int i=0; i<BUFFER_DIM ;kBuff[i++] = 0);
    for(int i=0; i<BUFFER_DIM ;sBuff[i++] = 0);
    readPosition = 0;
    writePosition = 0;
    keyCount = 0;
}

char getScanCode(char c){
    int i;
    for(i=0; i<CODES ;i++){
        if(KEYS[i][0] == c || KEYS[i][1] == c || SHIFTED[i][0] == c|| SHIFTED[i][1] == c)
            return SCAN_CODES[i];
    }
    return ERROR;
}