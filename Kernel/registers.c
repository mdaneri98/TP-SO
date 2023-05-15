#include <stdint.h>

/*extern void _getRegs();

static uint64_t REGISTERS[16] = {'0'};
//rdi,rsi,rdx,r10,r8,r9,rax,rbx,rcx,rbp,rsp,r11,r12,r13,r14,r15


void getRegisters(int err){
    _getregs();
}

void restore(){
    _restore(&REGISTERS);//llamo a funcion en asm que devuelve los registros a los valores en los que estaban antes del problema
                        cada vez que se llame a una fucnion se van a guardar los registros para facilitarlo
}

uint64_t writeValues(uint64_t rdi, uint64_t rsi,uint64_t rdx,uint64_t r10,uint64_t r8,uint64_t r9,uint64_t r15,uint64_t r14,uint64_t r13,
uint64_t r12,uint64_t r11,uint64_t rax,uint64_t rbp,uint64_t rcx,uint64_t rbx,uint64_t rsp)
    {
        REGISTERS[0] = rdi;
        REGISTERS[1] = rsi;
        REGISTERS[2] = rdx;
        REGISTERS[3] = r10;
        REGISTERS[4] = r8;
        REGISTERS[5] = r9;
        REGISTERS[6] = rax;
        REGISTERS[7] = rbx;
        REGISTERS[8] = rcx;
        REGISTERS[9] = rbp;
        REGISTERS[10] = rsp;
        REGISTERS[11] = r11;
        REGISTERS[12] = r12;
        REGISTERS[13] = r13;
        REGISTERS[14] = r14;
        REGISTERS[15] = r15;

        return &REGISTERS;
    }*/