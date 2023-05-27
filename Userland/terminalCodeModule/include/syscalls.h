#ifndef SYSCALLS_H
#define SYSCALLS_H

#include <color.h>

#define STDIN 0
#define STDOUT 1
#define STDERR 2

/* 
write: Escribe a pantalla el buffer con el color especificado.
Por ahora solo tenemos como fd STDIN y STDERR (igual que stdin pero con rojito)
*/
unsigned long _write(unsigned int fd, const char * buf, unsigned long count);

/* write: Vuelca los bytes ingresados en el buffer de teclado y devuelve la cantidad
 * de bytes reales escritos.
*/
unsigned int _read(unsigned int fd, char* buffer, int count);

void _sleep(unsigned long millis);
void _clear();
void _beep(unsigned long millis);
void _clock(int *hours, int *minutes, int *seconds);

void _getScreenH(uint16_t *height);
void _getScreenW(uint16_t *width);
void _gameRead(unsigned char *data);
void _getPtrToPixel(uint64_t x,uint64_t y, Color *c);
void _drawLine(uint16_t fromX, uint16_t fromY, uint16_t toX, uint16_t toY, Color *color);
void _getPenX(uint16_t *penX);
void _getPenY(uint16_t *penY);
void _changeFont(uint64_t newSize);

void _memoryDump(uint64_t *direction, uint8_t buffer[]);
int _getRegs(uint64_t buffer[]);

void _sysExecve(void* function, int argc, char *const argv[]);
void _sysFork();

#endif