#include <stdint.h>
#include <video.h>
#include <keyboard.h>
#include <string.h>
#include <programs.h>
#include <tester.h>

#define CODES 56
#define LETTERS 25
#define PRINT 47
#define R_SHIFT 0x36
#define L_SHIFT 0x2A
#define ERR_MSG "UNMAPPED KEY REQUESTED"
#define CMD_MSG ":IS NOT A COMMAND"
#define BUFFER 100


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
                                {' ',' '},{'n','n'},{'b','b'},{'c','c'},{'t','t'},{'u','u'},{'l','l'},{'r','r'},{'d','d'}};

const char SHIFTED[CODES][2] = {    {'A','a'},{'B','b'},{'C','c'},{'D','d'},{'E','e'},{'F','f'},{'G','g'},{'H','h'},{'I','i'},{'J','j'},{'K','k'},{'L','l'},{'M','m'},{'N','n'},{'O','o'},
                                    {'P','p'},{'Q','q'},{'R','r'},{'S','s'},{'T','t'},{'U','u'},{'V','v'},{'W','w'},{'X','x'},{'Y','y'},{'Z','z'},
                                    {'!','!'},{'@','@'},{'#','#'},{'$','$'},{'%','%'},{'^','^'},{'&','&'},{'*','*'},{'(','('},{')',')'},
                                    {'_','_'},{'+','+'},{'{','{'},{'}','}'},{':',':'},{'"','"'},{'~','~'},{'|','|'},{'<','<'},{'>','>'},{'?','?'},
                                    {' ',' '},{'n','n'},{'b','b'},{'c','c'},{'t','t'},{'u','u'},{'l','l'},{'r','r'},{'d','d'}};

const unsigned char SCAN_CODES[CODES] = {   0x1E,0x30,0x2E,0x20,0x12,0x21,0x22,0x23,0x17,0x24,0x25,0x26,0x32,0x31,0x18,
                                            0x19,0x10,0x13,0x1F,0x14,0x16,0x2F,0x11,0x2D,0x15,0x2C,
                                            0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,
                                            0x0C,0x0D,0x1A,0x1B,0x27,0x28,0x29,0x2B,0x33,0x34,0x35,
                                            0x39,0x1C,0x0E,0x3A,0x0F,0x48,0x4B,0x4D,0x50};


static void (*funcs[])() ={clock, clear, sleepTerminal, beep, help};

int LOCK = 0;
int SHIFT = 0;
char LINE[BUFFER] = "\0";
int LINE_LENGTH = 0;
/*SCAN CODES
0x01	esc         	0x02	1       	0x03	2       0x04	3       	    0x05	4       	0x06	5 	        (implementada)
0x07	6               0x08	7       	0x09	8 	    0x0A	9       	    0x0B	0           0x0C	-           (implementada)
0x0D	=       	    0x0E	backspace 	0x0F	tab     0x10	Q 	            0x11	W       	0x12	E           (implementada)
0x13	R               0x14	T 	        0x15	Y 	    0x16	U 	            0x17	I           0x18	O           (implementada)	
0x19	P 	            0x1A	[ 	        0x1B	]       0x1C	enter 	        0x1D	left ctrl	0x1E	A 	        (left ctrl missing)
0x1F	S               0x20	D       	0x21	F      	0x22	G           	0x23	H           0x24	J 	        (implementada)
0x25	K 	            0x26	L 	        0x27	 ;      0x28	' 	            0x29	` 	        0x2A	left shift  (implemented)
0x2B	\               0x2C	Z 	        0x2D	X 	    0x2E	C 	            0x2F	V           0x30	B           (implemented)	
0x31	N 	            0x32	M           0x33	,       0x34	. 	            0x35	/ 	        0x36	right shift (implemented)
0x39	space 	        0x3A	Caps        0x48    UP      0x4B    Left            0x4D    Right       0x50    Down        (implemented)
Down: 0x50


THE FOLLOWING KEYS WILL NOT BE IMPLEMENTED
0x38	left alt        0x3B	F1          0x3C	F2	    0x3D	F3 	            0x3E	F4 	        0x3F	F5          
0x40	F6 	            0x41	F7          0x42	F8 	    0x43	F9              0x44	F10 	    0x57	F11         
0x45	NumLock         0x46	ScrollLock

KEYPAD:
0x47	 7              0x48	 8 	        0x49    9 	    0x4A	 - 	            0x4B	 4          0x4C	 5 	
0x4D	 6 	            0x4E	 +      	0x4F	1       0x50	 2 	            0x51	 3	        0x52	 0 	
0x53	 .              0x37	 * 
*/

void keyboard_handler() {
    if (hasKey()) {
        read();
    }
}

int isMapped(unsigned char key){
    for(int i = 0; i < CODES;i++){
        if(SCAN_CODES[i] == key){
            return i;
        }
        
    }
    return -1;
}

void MapError(){
   scr_print(ERR_MSG);
}

void tab(){
    for(int i = 0;i < 4;i++){
        scr_printChar(' ');
    }
}

int lockChange(int current){
    if(current == 1){
        return 0;//apaga caps lock
    }
    return 1;//prende caps lock
}

void enter(){
    scr_printNewline();
    if(LINE_LENGTH != 0){
    run();
    }
    scr_print(PROMPT);
}

int getLength(){
    return LINE_LENGTH;
}

int isCommand(){
    int ans = -1;
    for(int i = 0;i < COMMANDS && ans == -1; i++){
        if(stringnCompare(LINE,CMDS[i],stringLength(CMDS[i]))== 0){
            ans = i;
        }
    }
    return ans;           
}

void run(){
    LINE[LINE_LENGTH] = '\0';
    int idx = isCommand();
    if(idx == -1){
        scr_print(LINE);
        scr_print(CMD_MSG);
        scr_printNewline();
        
    }else{
        (*funcs[idx])();
    }
    for(int i = 0; i < LINE_LENGTH;i++){
        LINE[i] = '\0';
        }
        LINE_LENGTH = 0;
}

void shift(){
    if(SHIFT == 1){
        SHIFT = 0;
    }else{
        SHIFT = 1;
    }
}

void capsLock(){
    if(LOCK == 1){
        LOCK = 0;
        //scr_print("LOCK DESACTIVADO");
    }else{
        LOCK = 1;
        // scr_print("LOCK ACTIVADO");
    }
}


void backspace(){
    if(LINE_LENGTH > 0){
        scr_backspace();
          LINE[LINE_LENGTH] = '\0';
        LINE_LENGTH -= 1;
    }
}


int toNum(int idx){
    return charToNum(LINE[idx]);
}

void read(){
    unsigned char key = readKey();
    char item;
    //verificar si es un caracter mapeado
    if(key > 0x80){
        if((key == R_SHIFT + 0x80) || (key == L_SHIFT + 0x80)){
            shift();
        }
        return;
    }else if((key == R_SHIFT) || (key == L_SHIFT)){
        shift();
        return;
    }
    int idx = isMapped(key);
    if(idx == -1){
        scr_print(ERR_MSG);
    }
    if(SHIFT){
        item = SHIFTED[idx][LOCK];
    }else{
        item = KEYS[idx][LOCK];
    }
    if(idx > PRINT){
        if(item == 'n'){
            enter();
        }else if(item == 'b'){
            backspace();
        }else if(item == 'c'){
            capsLock();
        }else if(item == 't' ){
            if(SHIFT == 1){
                return;
            }
            tab();       
        }else if(item =='u'){
            draw();
            return;
        }else if(item =='l'){
            draw();
            return;
        }else if(item =='r'){
            draw();
            return;
        }else if(item =='d'){
            draw();
            return;
        }
    }else{
        scr_printChar(item);
        toLower(&item);
        LINE[LINE_LENGTH++] = item;
    }
}



