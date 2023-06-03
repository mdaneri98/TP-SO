#ifndef __MEMORY_H_
#define __MEMORY_H_

#include <stdint.h>


#define REGISTERS_COUNT 17

/**
 * @brief Vuelca en buffer los valores de los registros anteriormente guardados.
 * 
 * @param buffer : Debe ser minimo de 16 bytes
 * @return int   : Si no hubo un guardado de registros anteriormente, se retorna 0. Caso contrario, 1. 
 */
int getRegistersInfo(uint64_t* buffer);
void printMemory(uint64_t *direction, uint8_t buffer[]);


#endif