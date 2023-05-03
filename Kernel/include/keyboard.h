#ifndef _KEYBOARD_H_
#define _KEYBOARD_H_

#define COMMANDS 5

static char *CMDS[COMMANDS] = {"clock","clear","sleep","beep","help"};

// char kgFlag();   // No usar ya que lee mediante pooling.
unsigned int hasKey();  // 1 si hay key esperando. 0 si nó.
unsigned char readKey();
void keyboard_handler();
void console();
void enter();
void tab();
void capsLock();
void read();
void shift();
void run();
int isCommand();
void backspace();
int getLength();
int toNum(int idx);
#endif