#include <memory.h>
#include <stdint.h>
#include <video.h>

extern uint64_t const inforeg[REGISTERS_COUNT];
extern uint8_t hasRegDump;

int getRegistersInfo(uint64_t* buffer) {
    if(!hasRegDump){
        return 0;
    }

    for (int i = 0; i < REGISTERS_COUNT; i++) {
        buffer[i] = inforeg[i];
    }
    return 1;
}

void savereg(uint64_t * regInfo) {
    // backup = regInfo;
    // madeBackup = 1;
}

void printMemory(uint64_t *direction, uint8_t buffer[32]) {
    uint8_t *memory = (uint8_t *) direction;
    for(int i=0; i<32 ;i++){
        buffer[i] = memory[i];
    }
}