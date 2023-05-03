#include <tester.h>
#include <video.h>

void draw(){
    uint64_t x = scr_getPenX();
    uint64_t y = scr_getPenY();
    Color col = {0xDD,0xDD,0xAA};
    scr_drawLine(x,y,x + 0x111,y+0x0000002,col);
}