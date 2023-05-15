/*
ESTE ES EL HEADER QUE DEBE PERMANECER EN EL KERNEL SPACE, ESPERA ARGUMENTOS DISTINTOS, YA QUE
TODAS LAS SYSCALLS ACÁ PARA MANTENER LA COMPATIBILIDAD CON EL VECTOR DE PUNTEROS A FUNCIÓN DEBEN
RECIBIR LA MISMA CANTIDAD Y TIPO DE ARGUMENTO, POR LO TANTO EL PROTOTIPO VA A SER DISTINTO AL DEL
HEADER QUE VA A ESTAR DISPONIBLE EN EL USER SPACE.
*/
#ifndef _KERNEL_SYSCALLS_H_
#define _KERNEL_SYSCALLS_H_

static uint64_t arqSysRead(uint64_t buff, uint64_t dim, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4);

static uint64_t arqSysWrite(uint64_t fd, uint64_t buff, uint64_t count, uint64_t nil1, uint64_t nil2, uint64_t nil3);

static uint64_t arqSysClear(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);

static uint64_t arqSysBeep(uint64_t millis, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5);

static uint64_t arqSysSleep(uint64_t millis, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5);

static uint64_t arqSysClock(uint64_t hours, uint64_t minutes, uint64_t seconds, uint64_t nil1, uint64_t nil2, uint64_t nil3);

typedef uint64_t (*SyscallVec)(uint64_t arg0, uint64_t arg1, uint64_t arg2, uint64_t arg3, uint64_t arg4, uint64_t arg5);

#endif // _KERNEL_SYSCALLS_H_