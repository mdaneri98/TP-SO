#include <stdint.h>
#include <video.h>
#include <kernelSyscalls.h>
#include <string.h>

uint64_t arqSysRead(uint64_t fd, uint64_t buff, uint64_t dim, uint64_t nil1, uint64_t nil2, uint64_t nil3);

uint64_t arqSysWrite(uint64_t buff, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5);

uint64_t arqSysClear(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);

typedef uint64_t (*SyscallVec)(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

static char buffer[512] = {0};
static uint64_t bufferDim = 512;

// Funcion auxiliar para limpiar el buffer
static void clearBuff(){
    for(int i=0; i<bufferDim ;buffer[i++] = 0);
}

/*
IDEA: Por qué no intentamos poner algún sistema de permisos o algún sistema que gestione archivos para poder
darle al usuario la posibilidad de crear archivos y poder escribirlos ahí.
Esto nos daría la posibilidad de que pueda escribir desde su shell en archivos y no solo en pantalla
+ nos daría la chance de comunicar mejor los errores, dejandolos en un archivo escrito o algo asi
Me gustaria averiguar como implementar un sistema de directorios para poder darle la chance al usuario
de que pueda organizar todo desde ahi, + le daria mucha mas utilidad a la shell y se pareceria mucho más a
un bash.
*/

// VECTOR PARA LAS SYSCALLS
static SyscallVec syscalls[3];


// CADA SYSCALL LA METEN EN ESTE VECTOR, NO SE OLVIDEN DE AUMENTAR EL TAMAÑO DEL VECTOR PARA QUE NO SE ROMPA NADA
void set_SYSCALLS(){
    syscalls[0] = (SyscallVec) arqSysRead;
    syscalls[1] = (SyscallVec) arqSysWrite;
    syscalls[2] = (SyscallVec) arqSysClear;
}

uint64_t syscallsDispatcher(uint64_t nr, uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5) {    
    return syscalls[nr](arg0, arg1, arg2, arg3, arg4, arg5);
}

/*
CADA SYSCALL RECIBE TODAS LAS VARIABLES QUE PUEDE RECIBIR EL SYSCALLDISPATCHER, 
QUEDA EN NOSOTROS HACER EL CASTEO QUE CORRESPONDA PARA CADA FUNCION E IGNORAR EL RESTO DE VARIABLES
PARA PODER USAR EL VECTOR COMO FORMA DE ORGANIZAR LAS SYSCALLS
*/

uint64_t arqSysRead(uint64_t fd, uint64_t buff, uint64_t dim, uint64_t nil1, uint64_t nil2, uint64_t nil3){
    if(dim > 512)
        // Manejo de error
    clearBuff();
    stringCopy(buffer, dim, buff);
    return (uint64_t) buffer; // Hay que castearlo bien en el userspace
}


// Syswrite (Banco más la version de linux que incluye un file descriptor, más adelante con una discusión previa le metemos refactor)
uint64_t arqSysWrite(uint64_t buff, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5) {
    char * tmpBuff = (char *) buff;
    scr_print(tmpBuff);
}

// Clear
uint64_t arqSysClear(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6){
    scr_clear();
    return 0;
}


