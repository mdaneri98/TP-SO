#ifndef _PLAYER_H_
#define _PLAYER_H_

#include <color.h>

typedef struct player{
    int p;
    int HP;
    uint64_t xAxis;
    uint64_t yAxis;
    int move;
    Color col;
}player;


#endif