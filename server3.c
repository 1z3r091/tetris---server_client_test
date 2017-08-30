#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include "tetris.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

void error_handling(char *message);
void showSendError(SOCKET sock);

int main(int argc, char **argv)
{
    WSADATA wsaData;
    int iResult;

    SOCKET ClientSocket;
    SOCKET ListenSocket;

    struct sockaddr_in serv_addr;
    struct sockaddr_in clnt_addr;

    int iSendResult;
    // char recvbuf[DEFAULT_BUFLEN];
    // int recvbuflen = DEFAULT_BUFLEN;

    int clnt_addr_size;
    char message[30];//"Hello World!\n";

    int number_to_send = 0;
    int id = htonl(number_to_send);
    BOOL check;

    unsigned long nonblock = 1;
    struct sockaddr_in peer;
    Player player[PLAYER_NUM];



    if(argc!=2)
    {
        printf("Usage : %s <port>\n", argv[0]);
        exit(1);
    }

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        wprintf(L"WSAStartup failed: %d\n", iResult);
        return 1;
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family=AF_INET;
    serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    serv_addr.sin_port=htons(atoi(argv[1]));

    // Create a SOCKET for connecting to server
    ListenSocket=socket(AF_INET, SOCK_STREAM, 0);

    if(ListenSocket == INVALID_SOCKET)
    {
        printf("%d", WSAGetLastError());
        error_handling("socket() error");
        WSACleanup();
        return 1;
    }

    // Setup the TCP listening socket
    iResult = bind(ListenSocket, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if (iResult == SOCKET_ERROR)
    {
        error_handling("bind() error");
        WSACleanup();
        return 1;
    }

    iResult = listen(ListenSocket, SOMAXCONN);
    if (iResult == SOCKET_ERROR)
    {
        error_handling("listen() error");
        WSACleanup();
        return 1;
    }
    // non blocking setting
    // iResult = ioctlsocket(ListenSocket, FIONBIO, &nonblock);
    //if (iResult == SOCKET_ERROR) {
    //   printf("ioctl error");

    // Accept a client socket
    clnt_addr_size=sizeof(clnt_addr);
    ClientSocket = accept(ListenSocket, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
    if (ClientSocket == INVALID_SOCKET)
    {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    // send id
    iSendResult = send( ClientSocket, &id, (int)sizeof(id), 0 );
    if (iSendResult == SOCKET_ERROR)
    {
        showSendError(ClientSocket);
    }

    srand(time(NULL));
    setCursorType(NOCURSOR);
    int key_input_frame = 100;

    // send initial player structure
    player[id].shape = rand() % BLOCK_NUMBER;
    initializePlayerSetting(id,&player[id]);
    initializeBlockSetting(id,&player[id]);
    initializeBoard(id,&player[id]);
    copyGameBoard(id, &player[id]);

    iSendResult = send( ClientSocket, (char *)&player[id], sizeof(Player), 0 );
    if (iSendResult == SOCKET_ERROR)
    {
        showSendError(ClientSocket);
    }

    // Receive until the peer shuts down the connection
    // 이부분에서 서버 작업 처리

    do
    {
        // get message
        check = 0;
        iResult = recv(ClientSocket, message, sizeof(message)-1, 0);
        if (iResult == SOCKET_ERROR)
        {
            continue;
            if (WSAGetLastError() != WSAEWOULDBLOCK)
            {
                // continue;
            }
        }
        message[iResult] = '\0';
        printf("%s%d", message,check); // debug

        // check next space
        if (strcmp(message,"nextspace") == 0)
        {
            check = checkNextSpace(id,&player[id]);

            if (check == TRUE)
            {
                player[id].block_y += 1;
            }
            else if (check == FALSE)
            {

            }

            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                // showSendError(cid);
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //continue;
                }
            }
        }


        // check testfull
        else if (strcmp(message,"testfull") == 0)
        {
            check = testFull(id,&player[id]);

            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                // showSendError(cid);
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //continue;
                }
            }
            if (check == TRUE)
            {

            }
            else if (check == FALSE)
            {

            }

        }


        // check nextstage
        else if (strcmp(message,"nextstage") == 0)
        {
            check = checkNextStage(id,&player[id]);

            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                // showSendError(cid);
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //continue;
                }
            }

            if (check == TRUE)
            {
                initializeNextStage(id, &player[id]);
                initializeBoard(id, &player[id]);
            }
            else if (check == FALSE)
            {
                copyGameBoard(id, &player[id]);
            }

        }

        // check gameover
        else if (strcmp(message,"gameover") == 0)
        {
            player[id].shape = player[id].next_shape;
            initializeBlockSetting(id, &player[id]);
            check = checkGameOver(id,&player[id]);

            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                // showSendError(cid);
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //continue;
                }
            }

            if (check == FALSE)
            {
                initializeBoard(id, &player[id]);
                copyGameBoard(id, &player[id]);
            }
            else if (check == TRUE)
            {

            }


            iSendResult = send( ClientSocket, (char *)&player[id], sizeof(Player), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                showSendError(ClientSocket);
            }
        }

        // check checkrotate
        else if (strcmp(message,"checkrotate") == 0)
        {
            int temp_rotation;

            if (player[id].rotation + 1 > 3)
            {
                temp_rotation = 0;
            }
            else
            {
                temp_rotation = player[id].rotation + 1;
            }
            check = checkSpace(player[id].block_x, player[id].block_y, temp_rotation,id, &player[id]);

            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                // showSendError(cid);
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //continue;
                }
            }

            if (check == TRUE)
            {
                player[id].rotation = temp_rotation;
                iSendResult = send( ClientSocket, (char *)&player[id], sizeof(Player), 0 );
                if (iSendResult == SOCKET_ERROR)
                {
                    showSendError(ClientSocket);
                }
            }
            else if (check == FALSE)
            {

            }
        }

        // check checkleft
        else if (strcmp(message,"checkleft") == 0)
        {
            check = checkSpace(player[id].block_x-NEXT_SPACE, player[id].block_y, player[id].rotation,id,&player[id]);

            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                // showSendError(cid);
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //continue;
                }
            }

            if (check == TRUE)
            {
                player[id].block_x -= 2;
                iSendResult = send( ClientSocket, (char *)&player[id], sizeof(Player), 0 );
                if (iSendResult == SOCKET_ERROR)
                {
                    showSendError(ClientSocket);
                }
            }
            else if (check == FALSE)
            {

            }
        }

        // check checkright
        else if (strcmp(message,"checkright") == 0)
        {
            check = checkSpace(player[id].block_x+NEXT_SPACE, player[id].block_y, player[id].rotation,id,&player[id]);

            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                // showSendError(cid);
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //continue;
                }
            }

            if (check == TRUE)
            {
                player[id].block_x += 2;
                iSendResult = send( ClientSocket, (char *)&player[id], sizeof(Player), 0 );
                if (iSendResult == SOCKET_ERROR)
                {
                    showSendError(ClientSocket);
                }
            }
            else if (check == FALSE)
            {

            }
        }

        // check checkdown
        else if (strcmp(message,"checkdown") == 0)
        {
            check = checkNextSpace(id, &player[id]);

            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                // showSendError(cid);
                if (WSAGetLastError() != WSAEWOULDBLOCK)
                {
                    //continue;
                }
            }

            if (check == TRUE)
            {
                player[id].block_y += 1;
                iSendResult = send( ClientSocket, (char *)&player[id], sizeof(Player), 0 );
                if (iSendResult == SOCKET_ERROR)
                {
                    showSendError(ClientSocket);
                }
            }
            else if (check == FALSE)
            {

            }
        }

        // check checkspace
        else if (strcmp(message,"checkspace") == 0)
        {

            while( (check = checkNextSpace(id, &player[id])) )
            {
                player[id].block_y += 1;
            }

            iSendResult = send( ClientSocket, (char *)&player[id], sizeof(Player), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                showSendError(ClientSocket);
            }

        }
        // debug
        printf("%s%d-%d-", message,check,player[id].block_y);
        strcpy(message, "");

        int x, y;
        for ( x = 1; x < BOARD_WIDTH + 1; x++)
        {
            for ( y = 1; y < BOARD_HEIGHT + 1; y++)
            {
                printf("%d ", player[id].game_board[x][y]);
            }
            printf("\n");
        }
        // debug end

    }
    while (1);

    // shutdown the connection since we're done
    iResult = shutdown(ClientSocket, SD_SEND);
    if (iResult == SOCKET_ERROR)
    {
        printf("shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(ClientSocket);
        WSACleanup();
        return 1;
    }

    // cleanup
    printf("Disconnected");
    close(ClientSocket); /* 연결 종료 */
    WSACleanup();

    return 0;
}

void showSendError(SOCKET sock)
{
    printf("send failed: %d\n", WSAGetLastError());
    closesocket(sock);
    WSACleanup();
    exit(1);
}

void error_handling(char *message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);

}
