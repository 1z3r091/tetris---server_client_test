#include <stdio.h>
#include <time.h>
#include <windows.h>
#include <stdlib.h>
#include <winsock2.h>
#include "tetris.h"

// MACROS
// _DEBUG가 필요 없을 경우에는 주석 처리 합니다.
#define _DEBUG

#define UP 72
#define LEFT 75
#define RIGHT 77
#define DOWN 80
#define ESC 27
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20
#define BOARD_START_X 5
#define BOARD_START_Y 3
#define BLOCK_SHAPE_SIZE 3
#define BOARD_FRAME 0
#define BLOCK 1
#define EMPTY 2
#define FILLED_BLOCK 3
#define NEXT_SPACE 2
#define BLOCK_NUMBER 2

// GLOBAL VARIABLES

Point block_shape_coordinate[][4][4] = {
     { {0,0,1,0,2,0,-1,0}, {0,0,0,1,0,-1,0,-2}, {0,0,1,0,2,0,-1,0}, {0,0,0,1,0,-1,0,-2} },
     { {0,0,1,0,0,1,1,1}, {0,0,1,0,0,1,1,1}, {0,0,1,0,0,1,1,1}, {0,0,1,0,0,1,1,1} },};
     /*{ {0,0,-1,0,0,-1,1,-1}, {0,0,0,1,-1,0,-1,-1}, {0,0,-1,0,0,-1,1,-1}, {0,0,0,1,-1,0,-1,-1} },
     { {0,0,-1,-1,0,-1,1,0}, {0,0,-1,0,-1,1,0,-1}, {0,0,-1,-1,0,-1,1,0}, {0,0,-1,0,-1,1,0,-1} }, };
     */


char block_shape[][BLOCK_SHAPE_SIZE] = {"▩", "□", "  ", "■"};
// int x, y, i; // for loop variables
// int key_input_frame = 100;
BOOL check;
Player player[PLAYER_NUM];

// FUNCTION DEFINITION

void initializePlayerSetting(int nplayer, Player *player)
{
    player[nplayer].stage_level = 1; // initial stage level
    player[nplayer].stage_clear_cnt = player[nplayer].stage_level * 1; // clear line cnt limit for each stage level
    player[nplayer].clear_cnt = 0;
    player[nplayer].board_x = nplayer * 51;
    player[nplayer].stay_time = player[nplayer].frame_time = 15;
    player[nplayer].score = 0;
    player[nplayer].stage_level = 0;
}

// 로직
void copyGameBoard(int nplayer, Player *player)
{
    int x, y;
    for (x = 1; x < BOARD_WIDTH + 1; x++) {
        for (y = 1; y < BOARD_HEIGHT + 1; y++) {
            player[nplayer].copy_game_board[x][y] = player[nplayer].game_board[x][y];
        }
    }
}

// 뷰
void showNextStage(int nx, int nplayer, Player *player)
{
    // set color
    SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7);

    gotoXY(nx+BOARD_START_X+5,BOARD_START_Y+4);  printf("+-------------+");
    gotoXY(nx+BOARD_START_X+5,BOARD_START_Y+5);  printf("|  Stage %d    |", player[nplayer].stage_level);
    gotoXY(nx+BOARD_START_X+5,BOARD_START_Y+6);  printf("+-------------+");
    Sleep(3000);
    initializeBoard(nplayer,&player[nplayer]);
    drawBoard(player[nplayer].board_x, nplayer, &player[nplayer]);
    refreshSideBoard(player[nplayer].board_x, nplayer,&player[nplayer]);
}

// 뷰
void refreshSideBoard(int nx, int nplayer, Player *player)
{
    // set color
    SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7);

    gotoXY(BOARD_START_X+25+nx,BOARD_START_Y);     printf("                ");
    gotoXY(BOARD_START_X+25+nx,BOARD_START_Y);     printf("Stage: %6d", player[nplayer].stage_level);
    gotoXY(BOARD_START_X+25+nx,BOARD_START_Y+1);   printf("                ");
    gotoXY(BOARD_START_X+25+nx,BOARD_START_Y+1);   printf("Score: %6d", player[nplayer].score);
}

// 뷰
void showNextBlock(BOOL show, int nplayer, Player *player)
{
    int nx, ny;
    int i;
    nx = BOARD_START_X+30;
    ny = BOARD_START_Y+5;
    if (show == TRUE) {
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 15);
        for (i = 0; i < 4; i++) {
            gotoXY(player[nplayer].board_x + nx + block_shape_coordinate[player[nplayer].next_shape][0][i].x*(BLOCK_SHAPE_SIZE-1), ny + block_shape_coordinate[player[nplayer].next_shape][0][i].y);
            printf("%s", block_shape[FILLED_BLOCK]);
        }
    } else if (show == FALSE) {
        for (i = 0; i < 4; i++) {
            gotoXY(player[nplayer].board_x + nx + block_shape_coordinate[player[nplayer].next_shape][0][i].x*(BLOCK_SHAPE_SIZE-1), ny + block_shape_coordinate[player[nplayer].next_shape][0][i].y);
            printf("%s", block_shape[EMPTY]);
        }
    }
}

// 뷰
void showStartMenu(void)
{
    int x,y,frame_time;

    frame_time = 100;
    x = 5;
    y = 3;

    while (TRUE) {
        // set color
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), x + 5);
        gotoXY(x,y);   printf("#######   #######   #######   #######   ###   #######");
        Sleep(frame_time);
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), x + 6);
        gotoXY(x,y+1); printf("   #      #            #      #     #    #    #");
        Sleep(frame_time);
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), x + 7);
        gotoXY(x,y+2); printf("   #      #######      #      #######    #    #######");
        Sleep(frame_time);
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), x + 8);
        gotoXY(x,y+3); printf("   #      #            #      #     #    #          #");
        Sleep(frame_time);
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), x + 9);
        gotoXY(x,y+4); printf("   #      #######      #      #     #   ###   #######");
        Sleep(frame_time);
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), x - 1);
        gotoXY(x+28,y+6);printf("               PLAYER 1                   ");
        gotoXY(x+28,y+7);printf("  ▲    : Rotate          Space : Hard Drop");
        gotoXY(x+28,y+8);printf("◀  ▶  : Left / Right"  );
        gotoXY(x+28,y+9);printf("  ▼    : Soft Drop       ESC : Quit");
        SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), x - 2);
        gotoXY(x+28,y+11);printf("               PLAYER 2                   ");
        gotoXY(x+28,y+12);printf("  T    : Rotate          A : Hard Drop");
        gotoXY(x+28,y+13);printf("F   H  : Left / Right"  );
        gotoXY(x+28,y+14);printf("  G    : Soft Drop       ESC : Quit");
        for (;;) {
            if (kbhit()) {
                getch();
                SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 7);
                system("cls");
                return;
            }
            SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), x - 3);
            gotoXY(x+15,y+17); printf("PRESS ANY KEY TO START");
            Sleep(frame_time+300);
            gotoXY(x+15,y+17); printf("                      ");
            Sleep(frame_time+300);
        }
    }
}

// 뷰
void showGameOver(int nx, int ny, int nplayer, Player *player)
{
    gotoXY(nx,ny);   printf("+---------------------------+");
    gotoXY(nx,ny+1); printf("|     Your Score : %6d   |", player[nplayer].score);
    gotoXY(nx,ny+2); printf("|                           |");
    gotoXY(nx,ny+3); printf("|        GAME OVER          |");
    gotoXY(nx,ny+4); printf("|                           |");
    gotoXY(nx,ny+5); printf("| Press any key to restart! |");
    gotoXY(nx,ny+6); printf("|                           |");
    gotoXY(nx,ny+7); printf("+---------------------------+");

    for (;;) {
        if ((GetAsyncKeyState(VK_RETURN) & 0x8000) && nplayer == ONE) {
            break;
        } else if ((GetAsyncKeyState(VK_SHIFT) & 0x8000) && nplayer == TWO) {
            break;
        }
    }
}

// 로직
void initializeBlockSetting(int nplayer, Player *player)
{
    // 블럭 모양 설정
    player[nplayer].shape = rand() % BLOCK_NUMBER;
    player[nplayer].next_shape = rand() % BLOCK_NUMBER;
    player[nplayer].rotation = 0;
    player[nplayer].block_x = 10;
    player[nplayer].block_y = 4;
}

// 뷰
void showCurrentnNextBlock(int nplayer, Player *player)
{
    showBlock(TRUE,player[nplayer].board_x,&player[nplayer]);
    showNextBlock(TRUE,player[nplayer].board_x, &player[nplayer]);
}

// 로직
BOOL checkGameOver(int nplayer, Player *player)
{
    if (checkSpace(player[nplayer].block_x, player[nplayer].block_y, player[nplayer].rotation,nplayer,&player[nplayer])) {
        return TRUE;
    }
    return FALSE;
}

// 뷰
void gameStart(int nplayer, SOCKET sock, Player *player)
{
    int iResult;
    BOOL check;

    // 블럭 한칸씩 아래로 이동
    if (--player[nplayer].stay_time == 0) {
        player[nplayer].stay_time = player[nplayer].frame_time;
        // 로직
        // check = checkNextSpace(nplayer, &player[nplayer]);
        iResult = recv(sock, &check, sizeof(check),0);
        if (iResult == SOCKET_ERROR)
            printf("ERROR");
        printf("%d ", check);

        if (check) { // 이동 중 더이상 내려갈 공간 없다 -> break
            showNextBlock(FALSE,nplayer,&player[nplayer]);
            player[nplayer].shape = player[nplayer].next_shape;
            initializeBlockSetting(nplayer, &player[nplayer]);
            showCurrentnNextBlock(nplayer, &player[nplayer]);

            if (checkGameOver(nplayer,&player[nplayer])) {
                // 뷰
                showGameOver(player[nplayer].board_x+3,7,nplayer, &player[nplayer]);
                initializeBoard(nplayer, &player[nplayer]);
                drawCompleteBoard(nplayer,&player[nplayer]);
                copyGameBoard(nplayer, &player[nplayer]);
                showNextBlock(TRUE,nplayer,&player[nplayer]);
            }
        }
        moveBlock(nplayer, &player[nplayer]);
    }

    // 키보드 입력 읽기
    // 로직
    if (keyInput(nplayer, &player[nplayer])) { // 입력 도중 더이상 내려갈 공간 없다 -> break
        showNextBlock(FALSE, nplayer, &player[nplayer]);
        player[nplayer].shape = player[nplayer].next_shape;
        initializeBlockSetting(nplayer, &player[nplayer]);
        showCurrentnNextBlock(nplayer, &player[nplayer]);

        if(checkGameOver(nplayer,&player[nplayer])) {
            // 뷰
            showGameOver(player[nplayer].board_x+3,7,nplayer, &player[nplayer]);
            initializeBoard(nplayer, &player[nplayer]);
            drawCompleteBoard(nplayer, &player[nplayer]);
            copyGameBoard(nplayer, &player[nplayer]);
            // 뷰
            showNextBlock(TRUE,nplayer,&player[nplayer]);
            player[nplayer].stay_time = player[nplayer].frame_time;
        }
    }
    Sleep(30);
}

void setCursorType(CURSOR_TYPE c)
{
     CONSOLE_CURSOR_INFO CurInfo;

     switch (c) {
     case NOCURSOR:
          CurInfo.dwSize=1;
          CurInfo.bVisible=FALSE;
          break;
     case SOLIDCURSOR:
          CurInfo.dwSize=100;
          CurInfo.bVisible=TRUE;
          break;
     case NORMALCURSOR:
          CurInfo.dwSize=20;
          CurInfo.bVisible=TRUE;
          break;
     }
     SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE),&CurInfo);
}

void gotoXY(int x, int y)
{
     COORD Cur;
     Cur.X=x;
     Cur.Y=y;
     SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),Cur);
}

// 로직
void initializeBoard(int nplayer, Player *player)
{
    int x,y;

     for (x = 0; x < BOARD_WIDTH+2; x++) {
        for (y = 0; y <  BOARD_HEIGHT+2; y++) {
            if (y == 0 || x == 0 || y == BOARD_HEIGHT+1 || x == BOARD_WIDTH+1) {
                player[nplayer].game_board[x][y] = BOARD_FRAME;
            } else {
                player[nplayer].game_board[x][y] = EMPTY;
            }
        }
     }

}

// 뷰
void drawCompleteBoard(int nplayer, Player *player)
{
    int x, y;
    system("cls");
    for (x = 0; x < BOARD_WIDTH+2; x++) {
        for (y = 0; y <  BOARD_HEIGHT+2; y++) {
            if (player[nplayer].game_board[x][y] == BOARD_FRAME) {
                gotoXY(BOARD_START_X+x*(BLOCK_SHAPE_SIZE-1)+player[nplayer].board_x,BOARD_START_Y+y);
                printf("%s", block_shape[BOARD_FRAME]);
            }
        }
    }

    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y);     printf("Stage: %6d", player[nplayer].stage_level);
    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y+1);   printf("Score: %6d", player[nplayer].score);
    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y+2);   printf("Next Shape");
    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y+3);   printf("+============+");
    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y+4);   printf("|            |");
    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y+5);   printf("|            |");
    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y+6);   printf("|            |");
    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y+7);   printf("|            |");
    gotoXY(BOARD_START_X+25+player[nplayer].board_x,BOARD_START_Y+8);   printf("+============+");
}

// 뷰
void drawBoard(int nx, int nplayer, Player *player)
{
    int x, y;
    for (x = 1; x < BOARD_WIDTH+1; x++) {
        for (y = 1; y <  BOARD_HEIGHT+1; y++) {
            if (player[nplayer].game_board[x][y] == BLOCK) {
                gotoXY(nx + BOARD_START_X+x*(BLOCK_SHAPE_SIZE-1),BOARD_START_Y+y);
                printf("%s", block_shape[BLOCK]);
            } else if (player[nplayer].game_board[x][y] == FILLED_BLOCK && player[nplayer].copy_game_board[x][y] != player[nplayer].game_board[x][y]) {
                SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), player[nplayer].shape + 10);
                gotoXY(nx + BOARD_START_X+x*(BLOCK_SHAPE_SIZE-1),BOARD_START_Y+y);
                printf("%s", block_shape[FILLED_BLOCK]);
            } else if (player[nplayer].game_board[x][y] == EMPTY) {
                gotoXY(nx + BOARD_START_X+x*(BLOCK_SHAPE_SIZE-1),BOARD_START_Y+y);
                printf("%s", block_shape[EMPTY]);
            }
        }
    }
}
// 뷰

void showBlock(BOOL show, int nplayer, Player *player)
{
    int i;
    if (show == TRUE) {
        for (i = 0; i < 4; i++) {
            gotoXY(player[nplayer].board_x+BOARD_START_X + block_shape_coordinate[player[nplayer].shape][player[nplayer].rotation][i].x*(BLOCK_SHAPE_SIZE-1) + player[nplayer].block_x, BOARD_START_Y +block_shape_coordinate[player[nplayer].shape][player[nplayer].rotation][i].y + player[nplayer].block_y);
            // set block color
            SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), player[nplayer].shape + 10);

            printf("%s", block_shape[BLOCK]);
        }
    } else if (show == FALSE) {
        for (i = 0; i < 4; i++) {
            gotoXY(player[nplayer].board_x+BOARD_START_X + block_shape_coordinate[player[nplayer].shape][player[nplayer].rotation][i].x*(BLOCK_SHAPE_SIZE-1) + player[nplayer].block_x, BOARD_START_Y +block_shape_coordinate[player[nplayer].shape][player[nplayer].rotation][i].y + player[nplayer].block_y);
            printf("%s", block_shape[EMPTY]);
        }
    }
}

// 로직
BOOL checkNextStage(int nplayer, Player *player)
{
    if (player[nplayer].clear_cnt >= player[nplayer].stage_clear_cnt) {
        return TRUE;
    }
    return FALSE;
}

// 로직
void initializeNextStage(int nplayer, Player *player)
{
    player[nplayer].clear_cnt = 0;
    player[nplayer].stage_level += 1;
    player[nplayer].frame_time /= 2; // move speed increase
}

// 로직
BOOL checkSpace(int x, int y, int rotation, int nplayer, Player *player)
{
    int i;
    int block_temp_x = x;
    int block_temp_y = y;
    int temp_rotation = rotation;

    // set block color
    SetConsoleTextAttribute( GetStdHandle( STD_OUTPUT_HANDLE ), 15);

    for (i = 0; i < 4; i++) {
        if (player[nplayer].game_board[(block_shape_coordinate[player[nplayer].shape][temp_rotation][i].x*(BLOCK_SHAPE_SIZE-1)+block_temp_x)/2][block_shape_coordinate[player[nplayer].shape][temp_rotation][i].y + block_temp_y] != EMPTY) {
            if (block_temp_y > player[nplayer].block_y) {
                if (testFull(nplayer, &player[nplayer])) {
                     // if next stage
                     if (checkNextStage(nplayer, &player[nplayer])) {
                        initializeNextStage(nplayer, &player[nplayer]);
                        refreshSideBoard(player[nplayer].board_x,nplayer, &player[nplayer]);
                        initializeBoard(nplayer, &player[nplayer]);
                        drawBoard(player[nplayer].board_x,nplayer, &player[nplayer]);
                        copyGameBoard(nplayer, &player[nplayer]);
                        showNextStage(player[nplayer].board_x, nplayer, &player[nplayer]);
                     }
                     // if not next stage
                     refreshSideBoard(player[nplayer].board_x,nplayer, &player[nplayer]);
                     drawBoard(player[nplayer].board_x, nplayer,&player[nplayer]);
                     copyGameBoard(nplayer, &player[nplayer]);
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

// 로직
BOOL checkNextSpace(int nplayer, Player *player)
{
    if (checkSpace(player[nplayer].block_x, player[nplayer].block_y+1, player[nplayer].rotation,nplayer,&player[nplayer])) {
        return TRUE;
    }
    return FALSE;
}

// 뷰
void moveBlock(int nplayer, Player *player)
{
    showBlock(FALSE,nplayer, &player[nplayer]);
    player[nplayer].block_y += 1;
    showBlock(TRUE,nplayer, &player[nplayer]);
}

// 로직
BOOL testFull(int nplayer, Player *player)
{
     int i,x,y;
     int temp_y;
     int combo = 1;

     for (i = 0; i < 4; i++) {
        player[nplayer].game_board[(block_shape_coordinate[player[nplayer].shape][player[nplayer].rotation][i].x*(BLOCK_SHAPE_SIZE-1)+player[nplayer].block_x)/2][block_shape_coordinate[player[nplayer].shape][player[nplayer].rotation][i].y + player[nplayer].block_y] = FILLED_BLOCK;
     }

     for (y = BOARD_HEIGHT; y > 1; y--) {
        for (x = 1; x < BOARD_WIDTH + 1; x++) {
            if (player[nplayer].game_board[x][y] == EMPTY) {
                return TRUE;
            }
        }
        if (x == BOARD_WIDTH + 1) {
            // score
            player[nplayer].score += combo * 100;
            combo++;

            // nextStage
            player[nplayer].clear_cnt += 1;

            for (temp_y = y; temp_y > 1; temp_y--) {
                for (x = 1; x < BOARD_WIDTH + 1; x++) {
                    player[nplayer].game_board[x][temp_y] = player[nplayer].game_board[x][temp_y-1];
                }
            }
            y++;
        }
     }
     return FALSE;
}

BOOL keyInput(int nplayer, Player *player)
{
    int key;
    int temp_rotation;
    int key_input_frame;

    while (key_input_frame >= 0) {
        key_input_frame--;
    }
    key_input_frame = 100;
    if (nplayer == ONE) {
        if (GetAsyncKeyState(VK_UP) & 0x8000) {
            if (player[nplayer].rotation + 1 > 3) {
                temp_rotation = 0;
            } else {
                temp_rotation = player[nplayer].rotation + 1;
            }
            if (checkSpace(player[nplayer].block_x, player[nplayer].block_y, temp_rotation,nplayer, &player[nplayer]) == FALSE) {
                showBlock(FALSE,nplayer,&player[nplayer]);
                player[nplayer].rotation = temp_rotation;
                showBlock(TRUE,nplayer,&player[nplayer]);
            }
        } else if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
            if (checkSpace(player[nplayer].block_x-NEXT_SPACE, player[nplayer].block_y, player[nplayer].rotation,nplayer,&player[nplayer]) == FALSE) {
                showBlock(FALSE,nplayer,&player[nplayer]);
                player[nplayer].block_x -= 2;
                showBlock(TRUE,nplayer,&player[nplayer]);
            }
        } else if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
            if (checkSpace(player[nplayer].block_x+NEXT_SPACE, player[nplayer].block_y, player[nplayer].rotation,nplayer,&player[nplayer]) == FALSE) {
                showBlock(FALSE,nplayer,&player[nplayer]);
                player[nplayer].block_x += 2;
                showBlock(TRUE,nplayer,&player[nplayer]);
            }
        } else if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
            if (checkSpace(player[nplayer].block_x, player[nplayer].block_y+1, player[nplayer].rotation,nplayer,&player[nplayer]) == FALSE) {
                moveBlock(nplayer, &player[nplayer]);
            }
        } else if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            while(checkNextSpace(nplayer,&player[nplayer]) == FALSE) {
                moveBlock(nplayer, &player[nplayer]);
            }
            return TRUE;
        }
    } else if (nplayer == TWO) {
        if (GetAsyncKeyState('T') & 0x8000) {
            if (player[nplayer].rotation + 1 > 3) {
                temp_rotation = 0;
            } else {
                temp_rotation = player[nplayer].rotation + 1;
            }
            if (checkSpace(player[nplayer].block_x, player[nplayer].block_y, temp_rotation,nplayer,&player[nplayer]) == FALSE) {
                showBlock(FALSE,nplayer,&player[nplayer]);
                player[nplayer].rotation = temp_rotation;
                showBlock(TRUE,nplayer,&player[nplayer]);
            }
        } else if (GetAsyncKeyState('F') & 0x8000) {
            if (checkSpace(player[nplayer].block_x-NEXT_SPACE, player[nplayer].block_y, player[nplayer].rotation,nplayer,&player[nplayer]) == FALSE) {
                showBlock(FALSE,nplayer,&player[nplayer]);
                player[nplayer].block_x -= 2;
                showBlock(TRUE,nplayer,&player[nplayer]);
            }
        } else if (GetAsyncKeyState('H') & 0x8000) {
            if (checkSpace(player[nplayer].block_x+NEXT_SPACE, player[nplayer].block_y, player[nplayer].rotation,nplayer,&player[nplayer]) == FALSE) {
                showBlock(FALSE,nplayer,&player[nplayer]);
                player[nplayer].block_x += 2;
                showBlock(TRUE,nplayer,&player[nplayer]);
            }
        } else if (GetAsyncKeyState('G') & 0x8000) {
            if (checkSpace(player[nplayer].block_x, player[nplayer].block_y+1, player[nplayer].rotation,nplayer,&player[nplayer]) == FALSE) {
                moveBlock(nplayer,&player[nplayer]);
            }
        } else if (GetAsyncKeyState('A') & 0x8000) {
            while(checkNextSpace(nplayer,&player[nplayer]) == FALSE) {
                moveBlock(nplayer,&player[nplayer]);
            }
            return TRUE;
        }
    }

    if (GetAsyncKeyState(VK_ESCAPE) & 0x8000) {
        system("cls");
        exit(0);
    }
    return FALSE;
}

