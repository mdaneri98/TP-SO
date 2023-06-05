#include <syscalls.h>
#include <string.h>

int memoryDump(int argsc, char* argsv[]) {
    uint8_t dump[32];
    unsigned long direction = 0;
    
    if(argsc == 0 || !isHex(argsv[1])){
        char *argErr = "You need to specify a memory direction in hex format.";
        printf(argErr);
        putChar('\n');
        return;
    } else{
        direction = stringHexToNum(argsv[1]);
    }

    _memoryDump((uint64_t *)direction, dump);
    
    printf("%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\n", 
    dump[0], dump[1], dump[2], dump[3], dump[4], dump[5], dump[6], dump[7],
    dump[8], dump[9], dump[10], dump[11], dump[12], dump[13], dump[14], dump[15]);

    printf("%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\t%x\n", 
    dump[16], dump[17], dump[18], dump[19], dump[20], dump[21], dump[22], dump[23],
    dump[24], dump[25], dump[26], dump[27], dump[28], dump[29], dump[30], dump[31]);

    return 0;
}
