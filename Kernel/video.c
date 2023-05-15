#include <video.h>
#include <font.h>
#include <string.h>
#include <lib.h>

static char buffer[64] = { '0' };

struct vbe_mode_info_structure {
    uint16_t attributes;        // deprecated, only bit 7 should be of interest to you, and it indicates the mode supports a linear frame buffer.
    uint8_t window_a;           // deprecated
    uint8_t window_b;           // deprecated
    uint16_t granularity;       // deprecated; used while calculating bank numbers
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;      // deprecated; used to switch banks from protected mode without returning to real mode
    uint16_t pitch;         // number of bytes per horizontal line
    uint16_t width;         // width in pixels
    uint16_t height;            // height in pixels
    uint8_t w_char;         // unused...
    uint8_t y_char;         // ...
    uint8_t planes;
    uint8_t bpp;            // bits per pixel in this mode
    uint8_t banks;          // deprecated; total number of banks in this mode
    uint8_t memory_model;
    uint8_t bank_size;      // deprecated; size of a bank, almost always 64 KB but may be 16 KB...
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;       // physical address of the linear frame buffer; write here to draw to the screen
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;   // size of memory in the framebuffer but not being displayed on the screen
    uint8_t reserved1[206];
} __attribute__ ((packed));

//Struct found at: https://wiki.osdev.org/VESA_Video_Modes and how to write and porgram in video mode as well

struct vbe_mode_info_structure* screenData = (void*)0x5C00; //Configurado en sysvar.asm

static uint16_t penX = 0, penY = 0;
static Color penColor = {0x7F, 0x7F, 0x7F};    // 3 bytes de color.
static uint8_t bytesPerPixel = 3;
static uint8_t fontSize = 1;

void* getPtrToPixel(uint16_t x, uint16_t y) {
    /*
    Punto (x,y) en la pantalla. Como la memoria es continua, para llegar a ese punto es mediante la sig. ecuación:
    */
    return (void*)(screenData->framebuffer + bytesPerPixel * (x + (y * (uint64_t)screenData->width)));
}


uint16_t scrGetWidth(void) {
    return screenData->width;
}

uint16_t scrGetHeight(void) {
    return screenData->height;
}

uint16_t scrGetPenX(void) {
    return penX;
}

uint16_t scrGetPenY(void) {
    return penY;
}

void scr_clear(void) {
    uint8_t *pos = (uint8_t*)((uint64_t)screenData->framebuffer);
    for (uint32_t len = bytesPerPixel * (uint32_t)screenData->width * screenData->height; len; len--, pos++)
        *pos = 0;
    penX = 0;
    penY = 0;
}

void scr_setPixel(uint16_t x, uint16_t y, Color color) {
    if (x >= screenData->width || y >= screenData->height)
        return;

    Color* pos = (Color*)getPtrToPixel(x, y);
    *pos = color;
}

void scr_drawRect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, Color color) {
    if (x >= screenData->width || y >= screenData->height)
        return;
    
    uint16_t maxWidth = screenData->width - x;
    if (width > maxWidth) width = maxWidth;
    
    uint16_t maxHeight = screenData->height - y;
    if (height > maxHeight) height = maxHeight;

    Color* ptr = (Color*)getPtrToPixel(x, y);
    unsigned int adv = screenData->width - width;
    for (int i=0; i<height; i++) {
        for (int c=0; c<width; c++)
            *(ptr++) = color;
        ptr += adv;
    }
}

void scrDrawLine(uint16_t fromX, uint16_t fromY, uint16_t toX, uint16_t toY, Color color) {
    unsigned int dx = toX < fromX ? (fromX - toX) : (toX - fromX);
    unsigned int dy = toY < fromY ? (fromY - toY) : (toY - fromY);
    
    // Lines can be drawn by iterating either horizontally or vertically. We check which way is best by comparing dx and dy.
    if (dy < dx) {
        // We draw the line iterating horizontally.
        // We ensure fromX < toX by swapping the points if necessary.
        if (fromX > toX) {
            uint16_t tmp = fromX;
            fromX = toX;
            toX = tmp;
            tmp = fromY;
            fromY = toY;
            toY = tmp;
        }

        if (fromX >= screenData->width) return;

        double m = (double)(toY - fromY) / (double)(toX - fromX);
        double b = fromY - m*fromX;
        if (toX >= screenData->width) toX = screenData->width - 1;

        for (uint16_t x = fromX; x <= toX; x++)
            scr_setPixel(x, (uint16_t)(m*x+b + 0.5), color);
    } else {
        // We draw the line iterating vertically.
        // We ensure fromY < toY by swapping the points if necessary.
        if (fromY > toY) {
            uint16_t tmp = fromX;
            fromX = toX;
            toX = tmp;
            tmp = fromY;
            fromY = toY;
            toY = tmp;
        }

        if (fromY >= screenData->height) return;

        double m = (double)(toX - fromX) / (double)(toY - fromY);
        double b = fromX - m*fromY;
        if (toY >= screenData->height) toY = screenData->height - 1;

        for (uint16_t y = fromY; y <= toY; y++)
            scr_setPixel((uint16_t)(m*y+b + 0.5), y, color);
    }
}

void scr_setPenPosition(uint16_t x, uint16_t y) {
    // We clamp the pen (x,y) to ensure there is enough space to draw a char in that position.
    uint16_t maxX = screenData->width - CHAR_WIDTH*fontSize;
    uint16_t maxY = screenData->height - CHAR_HEIGHT*fontSize;

    penX = x < maxX ? x : maxX;
    penY = y < maxY ? y : maxY;
}

void scr_setPenColor(Color color) {
    penColor = color;
}

void scr_printNewline(void) {
    penX = 0; // pen x is set to full left.

    // If there is space for another line, we simply advance the pen y. Otherwise, we move up the entire screen and clear the lower part.
    if (penY + (2*CHAR_HEIGHT*fontSize) <= screenData->height) {
        penY += CHAR_HEIGHT*fontSize;
    } else {
        void* dst = (void*)((uint64_t)screenData->framebuffer);

        // Apunta a la segunda línea de la pantalla. Linea ~ HEIGHT de un char.(son varias lineas de pixeles)
        void* src = (void*)(dst + bytesPerPixel * (CHAR_HEIGHT*fontSize * (uint64_t)screenData->width));

        // 3 * width * (toda la pantalla - una linea)
        uint64_t len = bytesPerPixel * ((uint64_t)screenData->width * (screenData->height - CHAR_HEIGHT*fontSize));
        
        memcpy(dst, src, len);
        memset(dst+len, 0, bytesPerPixel * (uint64_t)screenData->width * CHAR_HEIGHT*fontSize);
    }
}

void scrPrintStringWithColor(char *str, Color color){
    Color defaultColor = penColor;
    scr_setPenColor(color);
    scrPrint(str);
    scr_setPenColor(defaultColor);
}

// Lo tengo que terminar de implementar, el 
//cursor se mueve bien pero tengo que ver como pintar de negro todo el pixel
void scr_backspace(){
    if(penX < 0x00 && penY == CHAR_HEIGHT*fontSize) // Está en el inicio de la pantalla, no se puede hacer balckspace
        return;
    penX -= CHAR_WIDTH*fontSize;
    if (penX < 0x00){
        penX = screenData->width - CHAR_WIDTH*fontSize;
        penY = penY - CHAR_HEIGHT*fontSize;
    }
    Color black = {0x00, 0x00, 0x00};
    for (int h=0; h<CHAR_HEIGHT*fontSize; h++) {
        Color* pos = (Color*)getPtrToPixel(penX, penY+h);
        for(int i=0; i<CHAR_WIDTH*fontSize ;i++)
            pos[i] = black;
    }
}

void scr_printChar(char c) {
    if (c == '\n') {
        scr_printNewline();
        return;
    }
    if (c == '\b'){
        scr_backspace();
        return;
    }
    if(c == '\t'){
        scrPrint("   ");
        return;
    }
    
    if (c >= FIRST_CHAR && c <= LAST_CHAR) {
        int w, offset = 0, j;
	    char* data = font + 32*(c-33);
	    for (int h=0; h<CHAR_HEIGHT; h++) {
            offset = h*(fontSize-1);
            for(j=0; j<fontSize ;j++){
                char *auxData = data;
                Color* pos = (Color*)getPtrToPixel(penX, penY + h + offset--);
                w = 0;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b00000001) pos[w] = penColor;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b00000010) pos[w] = penColor;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b00000100) pos[w] = penColor;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b00001000) pos[w] = penColor;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b00010000) pos[w] = penColor;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b00100000) pos[w] = penColor;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b01000000) pos[w] = penColor;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b10000000) pos[w] = penColor;
                auxData++;
                for(int i=0; i<fontSize ;i++, w++)
                    if (*auxData & 0b00000001) pos[w++] = penColor;
            }
            data += 2;
    	}
    }

    penX += CHAR_WIDTH*fontSize;
    if (penX > screenData->width - CHAR_WIDTH*fontSize)
        scr_printNewline();
}

uint32_t scrPrint(char* s) {
    for (; *s != 0; s++)
		scr_printChar(*s);
    return penX | ((uint32_t)penY << 16);
}

/*uint32_t scr_println(char* s) {
    scrPrint(s);
    scr_printNewline();
    return penX | ((uint32_t)penY << 16);
}*/

void scr_printHex(uint64_t value){
    scr_printBase(value,16);
}

static uint32_t toBase(uint64_t value, char * buffer, uint32_t base)
{
	char *p = buffer;
	char *p1, *p2;
	uint32_t digits = 0;

	//Calculate characters for each digit
	do
	{
		uint32_t remainder = value % base;
		*p++ = (remainder < 10) ? remainder + '0' : remainder + 'A' - 10;
		digits++;
	}
	while (value /= base);

	// Terminate string in buffer.
	*p = 0;

	//Reverse string in buffer.
	p1 = buffer;
	p2 = p - 1;
	while (p1 < p2)
	{
		char tmp = *p1;
		*p1 = *p2;
		*p2 = tmp;
		p1++;
		p2--;
	}

	return digits;
}

void scr_printBase(uint64_t value, uint32_t base)
{
    toBase(value, buffer, base);
    scrPrint(buffer);
}

void scrChangeFont(uint64_t newSize){
    if(newSize > 0 && newSize < 6)
        fontSize = newSize;
}