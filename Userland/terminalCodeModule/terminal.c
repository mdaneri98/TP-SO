
#include <stdio.h>
#include <string.h>
#include <syscalls.h>
#include <terminal.h>
#include <tron.h>


// Source code begins here
void startTerminal() {

    _sysExecve(sh, 0, NULL);

}

