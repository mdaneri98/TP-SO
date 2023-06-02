#include <tron.h>
#include <syscalls.h>
#include <sh.h>
#include <player.h>

#define DOT  0x000001
#define DEF_HEIGHT 16
#define DEF_WIDTH 9

unsigned char player1[] =   {0x48,0x4B,0x4D,0x50};
                            //up,left,right,down
unsigned char player2[] =   {0x11,0x1E,0x20,0x1F};
                            //w,a,d,s

static int checkDir(int move1, int move2);
static void borders();
static void setup(player *p1,player*p2);
static void looseHP(player *p);
static int check(Color col);
static Color validate(uint64_t x,uint64_t y);
static void moveUp(player *p);
static void moveDown(player *p);
static void moveLeft(player *p);
static void moveRight(player *p);
static void movePlayers(player *p1,player *p2);
static void changeMove(unsigned char key,player *p1,player *p2);
static int checkDir(int move1, int move2);
static int findKey(unsigned char key);
static int who(player *p1,player *p2);
static void play(player *p1, player * p2);


void tron(){
    struct player p1;
    struct player p2;
    setup(&p1,&p2);
    play(&p1,&p2);
}

static void borders(){
    uint16_t scrW, scrY;
    _getScreenW(&scrW);
    _getScreenH(&scrY);
    uint16_t xLimit = scrW -DEF_WIDTH;
    uint16_t yLimit = scrY -DEF_HEIGHT;
    uint16_t x = DEF_WIDTH;
    uint16_t y = DEF_HEIGHT;
    Color col = {0x7F,0x7F,0x7F};
    _drawLine(x, y, xLimit, y, &col);
    _drawLine(x, y, x, yLimit, &col);
    _drawLine(x, yLimit, xLimit, yLimit, &col);
    _drawLine(xLimit, y, xLimit, yLimit, &col);
}

static void setup(player *p1,player*p2){
    _clear();
    _changeFont(1);
    uint16_t scrW, scrY;
    _getScreenW(&scrW);
    _getScreenH(&scrY);
    uint64_t xPoint = scrW/2;
    uint64_t yPoint = (scrY- DEF_HEIGHT)/2;
    borders();
    Color cp1 = {0x00, 0x00, 0xAA}; //RED
    Color cp2 = {0xAA, 0x00, 0x00}; //BLUE
    p1->p = 1;
    p2->p = 2;
    p1->move = 3;   //<---
    p2->move = 2;   //--->
    p1->col = cp1;
    p2->col = cp2;
    p1->HP = 2;
    p2->HP = 2;
    p1->yAxis = yPoint;
    p2->yAxis = yPoint + DEF_HEIGHT;
    p1->xAxis = xPoint - (3 * DEF_WIDTH);
    p2->xAxis = xPoint +(3 * DEF_WIDTH);
    p1->xAxis = xPoint - DOT;
    p2->xAxis = xPoint + DOT;
}


static void looseHP(player *p){
    p->HP -= 1;
}

static int check(Color col){
    Color black = {0x00, 0x00, 0x00};
    if(col.r != black.r || col.g != black.g || col.b != black.b){
        return 0;
    }
    return 1;
}

static Color validate(uint64_t x,uint64_t y){    
    Color c;
    _getPtrToPixel(x, y, &c);
    return c;
}

static void moveUp(player *p){
    int valid = check(validate(p->xAxis, p->yAxis + DOT));
    if(valid){
        _drawLine(p->xAxis,p->yAxis + DOT,p->xAxis,p->yAxis,&p->col);
        p->yAxis += DOT;
    }else
    looseHP(p);
}

static void moveDown(player *p){
     int valid = check(validate(p->xAxis, p->yAxis - DOT));
    if(valid){
        _drawLine(p->xAxis,p->yAxis - DOT,p->xAxis,p->yAxis,&p->col);
        p->yAxis -= DOT;
    }else
    looseHP(p);
}

static void moveLeft(player *p){
    int valid = check(validate(p->xAxis - DOT, p->yAxis));
    if(valid){
        _drawLine(p->xAxis - DOT,p->yAxis,p->xAxis,p->yAxis,&p->col);
        p->xAxis -= DOT;
    }else 
    looseHP(p);
}

static void moveRight(player *p){
    int valid = check(validate(p->xAxis + DOT, p->yAxis));
    if(valid){
        _drawLine(p->xAxis + DOT,p->yAxis,p->xAxis,p->yAxis,&p->col);
        p->xAxis += DOT;
    }else 
        looseHP(p);
}

static void movePlayers(player *p1,player *p2){
    int i = p1->move;
    int j = p2->move;
    if(i == 4){
        moveUp(p1);
    }else if(i ==2){
        moveLeft(p1);
    }else if(i == 3){
        moveRight(p1);
    }else if(i == 1){
        moveDown(p1);
    }
    if(j == 4){
        moveUp(p2);
    }else if(j==2){
        moveLeft(p2);
    }else if(j == 3){
        moveRight(p2);
    }else if(j == 1){
        moveDown(p2);
    }
}

static void changeMove(unsigned char key,player *p1,player *p2){
    for(int i = 0;i < 4;i++){
        if(key == player1[i]){
            if(checkDir(p1->move, i + 1))
                p1->move = i+1;
            return;
        }else if(key == player2[i]){
            if(checkDir(p2->move, i + 1))
                p2->move = i+1;
            return;
        }
    }
    return;
}

static int checkDir(int move1, int move2){
    if(move1 == 1)
        return move2 != 4;
    if(move1 == 2)
        return move2 != 3;
    if(move1 == 3)
        return move2 != 2;
    if(move1 == 4)
        return move2 != 1;
    return -1;
}

static int findKey(unsigned char key){
    for(int i = 0;i < 4;i++){
        if(player1[i] == key || player2[i] == key){
            return 0;
        }
    }
    return 1;
}

static int who(player *p1,player *p2){
    if(p1->HP == 0){
        return 2;
    }else
        return 1;
}

static void play(player *p1, player * p2){
    while(p1->HP > 0 && p2->HP > 0){
        _sleep(80);
        unsigned char toDo;
        _gameRead(&toDo);
        if(findKey(toDo) == 0){
            changeMove(toDo,p1,p2);
        }
        movePlayers(p1,p2);
    }
    printf("GAME FINISHED: WINNER IS PLAYER %d",who(p1,p2));
    _beep(2000);
    _clear();   
}