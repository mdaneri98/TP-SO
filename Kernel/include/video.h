#ifndef _VIDEO_H_
#define _VIDEO_H_

#include <stdint.h>

/* The width of a character in pixels. */
#define CHAR_WIDTH 9
/* The height of a character in pixels. */
#define CHAR_HEIGHT 16

#define PROMPT "(^: "

typedef struct {
	uint8_t b;
	uint8_t g;
	uint8_t r;
} Color;


void * getPtrToPixel(uint16_t x, uint16_t y);
/* Gets the width of the screen in pixels. */
uint16_t scrGetWidth(void);

/* Gets the height of the screen in pixels. */
uint16_t scrGetHeight(void);

/* Gets the X position of the writting pen. */
uint16_t scrGetPenX(void);

/* Gets the Y position of the writting pen. */
uint16_t scrGetPenY(void);
/* Clears the whole screen to black. */
void scr_clear(void);

/* Sets the color of a specific pixel on the screen. */
void scr_setPixel(uint16_t x, uint16_t y, Color color);

/* Sets a specified rectangle of pixels on the screen to the specified color. */
void scr_drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color color);

void scrDrawLine(uint16_t fromX, uint16_t fromY, uint16_t toX, uint16_t toY, Color color);


/* Sets the pen position for drawing characters on the screen as a console. */
void scr_setPenPosition(uint16_t x, uint16_t y);

/* Sets the pen color for drawing characters on the screen as a console. */
void scr_setPenColor(Color color);

/* Advances the pen to the beginning of the next line. */
void scr_printNewline(void);

/* Prints a single character with the pen, wrapping around the end of the screen and pushing old lines up if necessary. */
void scr_printChar(char c);

void scr_backspace();

/* Prints a string of characters with the pen, wrapping around the end of the screen and pushing old lines up if necessary.
Returns the new pen position as a 32 bit number, where the 16 lowest bits are the x and the upper 16 bits are the y. */
uint32_t scrPrint(char* s);

/* Prints a string of characters with scrPrint(s) followed by a newline.
Returns the new pen position as a 32 bit number, where the 16 lowest bits are the x and the upper 16 bits are the y. 
uint32_t scr_println(char* s);*/

void scrPrintStringWithColor(char *str, Color color);

void scr_printBase(uint64_t value, uint32_t base);
void scr_printHex(uint64_t value);
void console();
void scrChangeFont(uint64_t newSize);

#endif