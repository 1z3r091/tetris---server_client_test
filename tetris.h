#ifndef TETRIS_H_INCLUDED
#define TETRIS_H_INCLUDED

#include <stdio.h>
#include <time.h>
#include <windows.h>

// MACROS
// _DEBUG가 필요 없을 경우에는 주석 처리 합니다.
#define _DEBUG

#define UP 72
#define LEFT 75
#define RIGHT 77
#define DOWN 80
#define ESC 27
#define BOARD_START_X 5
#define BOARD_START_Y 3
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define BLOCK_SHAPE_SIZE 3
#define BOARD_FRAME 0
#define BLOCK 1
#define EMPTY 2
#define FILLED_BLOCK 3
#define NEXT_SPACE 2
#define BLOCK_NUMBER 2
#define ONE 0
#define TWO 1
#define PLAYER_NUM 2

typedef struct
{
    int x;
    int y;
} Point;

typedef struct
{
    int shape;
    int next_shape;
    int rotation;
    int coordinate;
    int game_board[BOARD_WIDTH+2][BOARD_HEIGHT+2];
    int copy_game_board[BOARD_WIDTH+2][BOARD_HEIGHT+2];
    int block_x, block_y;
    int frame_time, stay_time;
    int score;
    int stage_level;
    int stage_clear_cnt;
    int clear_cnt;
    int board_x;
} Player;

// FUNCTION PROTOTYPE
typedef enum { NOCURSOR, SOLIDCURSOR, NORMALCURSOR } CURSOR_TYPE; // setcursortype func enum
void setCursorType(CURSOR_TYPE c); // cursor on/off func
void gotoXY(int x, int y); // x,y - coordinate change func

void initializeBoard(Player *player);
void drawCompleteBoard(Player *player);
void drawBoard(Player *player);
void showBlock(BOOL show, Player *player);
void showNextBlock(BOOL show, Player *player);
void moveBlock(Player *player);
BOOL checkSpace(int x, int y, int rotation, Player *player);
BOOL testFull(Player *player);
BOOL keyInput(int nplayer, Player *player, SOCKET sock);
void gameStart(int nplayer, SOCKET sock, Player *player);
void showStartMenu(void);
void showGameOver(int nplayer, Player *player);
void refreshSideBoard(Player *player);
void showNextStage(Player *player);
void copyGameBoard(Player *player);

void initializeBlockSetting(Player *player);
BOOL checkGameOver(Player *player);
void showCurrentnNextBlock(Player *player);
void initializePlayerSetting(int nplayer, Player *player);
BOOL checkNextSpace(Player *player);
BOOL checkNextStage(Player *player);
void initializeNextStage(Player *player);
void saveBlockinBoard(Player *player);
#endif // TETRIS_H_INCLUDED
