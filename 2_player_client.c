#include <stdio.h>
#include <stdlib.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include "tetris.h"

#pragma comment (lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void ErrorHandling(char* message);

int main(int argc, char* argv[])
{
    WSADATA wsaData;
    SOCKET ConnectSocket = INVALID_SOCKET;
    SOCKADDR_IN servAddr;
    int iResult;

    unsigned long nonblock = 1;
    struct sockaddr_in peer;

    char message[512] = {0,};
    //int id;
    char id;
    Player player[PLAYER_NUM];


    if(argc!=3)
    {
        printf("Usage : %s <IP> <port>\n", argv[0]);
        exit(1);
    }

    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        ErrorHandling("WSAStartup() errer!");
    }

    ConnectSocket=socket(AF_INET, SOCK_STREAM, 0);
    //if(ConnectSocket==INVALID_SOCKET)
    //	ErrorHandling("ConnectSocketet() error!");
    // ioctlsocket(ConnectSocket, FIONBIO, &nonblock);

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family=AF_INET;
    servAddr.sin_addr.s_addr=inet_addr(argv[1]);
    servAddr.sin_port=htons(atoi(argv[2]));

    // connect to server
    iResult = connect(ConnectSocket, (SOCKADDR*)&servAddr, sizeof(servAddr));
    if (iResult == SOCKET_ERROR)
    {
        ErrorHandling("connect() error!");
    }

    // receive id
    //iResult = recv(ConnectSocket, &id, (int)sizeof(id), 0);
    iResult = recv(ConnectSocket, &id, (int)sizeof(id), 0);
    if (iResult == SOCKET_ERROR)
    {
        ErrorHandling("connect() error!");
    }
    printf("Connecting to Server... %d\n", id);//ntohl(id));

    // receive Player structure
    iResult = recv(ConnectSocket, (char *)&player[id], sizeof(Player), 0);
    if (iResult == SOCKET_ERROR)
    {
        ErrorHandling("connect() error!");
    }
    Sleep(3000);
    system("cls");

    // Receive until the peer closes the connection
    // 이 부분에서 클라이언트 작업 처리
    srand(time(NULL));
    setCursorType(NOCURSOR);

    showStartMenu();
    drawCompleteBoard(&player[id]);
    showCurrentnNextBlock(&player[id]);

    do
    {
        gameStart(id, ConnectSocket, &player[id]);
    }
    while(1);


    // cleanup
    printf("Disconnected");
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}

void ErrorHandling(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}
