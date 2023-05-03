/*
ESTE ES EL HEADER QUE DEBE PERMANECER EN EL KERNEL SPACE, ESPERA ARGUMENTOS DISTINTOS, YA QUE
TODAS LAS SYSCALLS ACÁ PARA MANTENER LA COMPATIBILIDAD CON EL VECTOR DE PUNTEROS A FUNCIÓN DEBEN
RECIBIR LA MISMA CANTIDAD Y TIPO DE ARGUMENTO, POR LO TANTO EL PROTOTIPO VA A SER DISTINTO AL DEL
HEADER QUE VA A ESTAR DISPONIBLE EN EL USER SPACE.
*/
#ifndef _KERNEL_SYSCALLS_H_
#define _KERNEL_SYSCALLS_H_

uint64_t arqSysRead(uint64_t fd, uint64_t buff, uint64_t dim, uint64_t nil1, uint64_t nil2, uint64_t nil3);

uint64_t arqSysWrite(uint64_t buff, uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5);

uint64_t arqSysClear(uint64_t nil1, uint64_t nil2, uint64_t nil3, uint64_t nil4, uint64_t nil5, uint64_t nil6);

#endif // _KERNEL_SYSCALLS_H_