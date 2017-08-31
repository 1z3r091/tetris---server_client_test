#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <sys/types.h>
#include <time.h>
#include "tetris.h"

#pragma comment(lib, "ws2_32.lib")

#define DEFAULT_BUFLEN 512
#define DEFAULT_PORT "27015"

Player player[PLAYER_NUM];

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
    //int id = htonl(number_to_send);
    char id = 0;
    int cnt = 0;
    BOOL check;

    unsigned long nonblock = 1;
    struct sockaddr_in peer;
    Player player[PLAYER_NUM];
    // memset(&player[id],0,sizeof(Player))

    srand(time(NULL));
    setCursorType(NOCURSOR);
    int key_input_frame = 100;

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
    // ioctlsocket(ListenSocket, FIONBIO, &nonblock);

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
    //iResult = ioctlsocket(ListenSocket, FIONBIO, &nonblock);
    //if (iResult == SOCKET_ERROR)
    {
     //   printf("ioctl error");
    }

    typedef struct fd_set
    {
        u_int fd_count;               /* how many are SET? */
        SOCKET  fd_array[FD_SETSIZE];   /* an array of SOCKETs */
    } fd_set;

    struct timeval
    {
        long    tv_sec;         /* seconds */
        long    tv_usec;        /* and microseconds */
    };

    fd_set read, tmp;
    TIMEVAL time;

    FD_ZERO(&read);
    FD_SET(ListenSocket, &read);

    int re;
    int i;
    for (i = 0; i <= 1; i++)
    {
        player[i].shape = rand() % BLOCK_NUMBER;
        initializePlayerSetting(i,&player[i]);
        initializeBlockSetting(&player[i]);
        initializeBoard(&player[i]);
        copyGameBoard(&player[i]);
    }

    while(1)
    {
        tmp = read;
        time.tv_sec = 3;
        time.tv_usec = 0;
        int i;
        re = select(NULL, &tmp, NULL, NULL, &time);
        if(SOCKET_ERROR == re) break;
        if(0 == re) continue;
        int ids[2];
        for(i=0; i<read.fd_count; i++)
        {
            if(FD_ISSET(read.fd_array[i], &tmp))
            {
                if(ListenSocket == read.fd_array[i])
                {
                    SOCKADDR_IN caddr;
                    int size = sizeof(caddr);
                    SOCKET cid = accept(ListenSocket, (SOCKADDR*)&caddr, &size);
                    if (cid == SOCKET_ERROR)
                    {
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {

                        }
                    }

                    FD_SET(cid, &read);
                    printf("[%d]님이 접속하셧습니다\n", cid);
                    ids[cnt++] = cid;
                    // send id
                    if (cid == ids[0]) id = 0;
                    else if (cid == ids[1]) id = 1;
                    //iSendResult = send( cid, &id, (int)sizeof(id), 0 );
                    iSendResult = send( cid, &id, (int)sizeof(id), 0 );
                    if (iSendResult == SOCKET_ERROR)
                    {
                        //showSendError(cid);
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {

                        }
                    }

                    // send initial player structure


                    iSendResult = send( cid, (char *)&player[id], sizeof(Player), 0 );
                    if (iSendResult == SOCKET_ERROR)
                    {
                        // showSendError(cid);
                        if (WSAGetLastError() != WSAEWOULDBLOCK)
                        {

                        }
                    }


                }
                else
                {
                    if (read.fd_array[i] == ids[0]) id = 0;
                    else if (read.fd_array[i] == ids[1]) id = 1;
                    // get message
                    check = 0;
                    iResult = recv(read.fd_array[i], message, sizeof(message)-1, 0);
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
                        check = checkNextSpace(&player[id]);

                        if (check == TRUE)
                        {
                            player[id].block_y += 1;
                        }
                        else if (check == FALSE)
                        {

                        }

                        iSendResult = send( read.fd_array[i], &check, sizeof(check), 0 );
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
                        check = testFull(&player[id]);

                        iSendResult = send( read.fd_array[i], &check, sizeof(check), 0 );
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
                        check = checkNextStage(&player[id]);

                        iSendResult = send( read.fd_array[i], &check, sizeof(check), 0 );
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
                            initializeNextStage(&player[id]);
                            initializeBoard( &player[id]);
                        }
                        else if (check == FALSE)
                        {
                            copyGameBoard(&player[id]);
                        }

                    }

                    // check gameover
                    else if (strcmp(message,"gameover") == 0)
                    {
                        player[id].shape = player[id].next_shape;
                        initializeBlockSetting(&player[id]);
                        check = checkGameOver(&player[id]);

                        iSendResult = send( read.fd_array[i], &check, sizeof(check), 0 );
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
                            initializeBoard(&player[id]);
                            copyGameBoard(&player[id]);
                        }
                        else if (check == TRUE)
                        {

                        }


                        iSendResult = send( read.fd_array[i], (char *)&player[id], sizeof(Player), 0 );
                        if (iSendResult == SOCKET_ERROR)
                        {
                            showSendError(read.fd_array[i]);
                            // showSendError(cid);
                            if (WSAGetLastError() != WSAEWOULDBLOCK)
                            {
                                //continue;
                            }
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
                        check = checkSpace(player[id].block_x, player[id].block_y, temp_rotation, &player[id]);

                        iSendResult = send( read.fd_array[i], &check, sizeof(check), 0 );
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
                            iSendResult = send( read.fd_array[i], (char *)&player[id], sizeof(Player), 0 );
                            if (iSendResult == SOCKET_ERROR)
                            {
                                // showSendError(cid);
                                if (WSAGetLastError() != WSAEWOULDBLOCK)
                                {
                                    //continue;
                                }
                            }
                        }
                        else if (check == FALSE)
                        {

                        }
                    }

                    // check checkleft
                    else if (strcmp(message,"checkleft") == 0)
                    {
                        check = checkSpace(player[id].block_x-NEXT_SPACE, player[id].block_y, player[id].rotation,&player[id]);

                        iSendResult = send( read.fd_array[i], &check, sizeof(check), 0 );
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
                            iSendResult = send( read.fd_array[i], (char *)&player[id], sizeof(Player), 0 );
                            if (iSendResult == SOCKET_ERROR)
                            {
                                // showSendError(cid);
                                if (WSAGetLastError() != WSAEWOULDBLOCK)
                                {
                                    //continue;
                                }
                            }
                        }
                        else if (check == FALSE)
                        {

                        }
                    }

                    // check checkright
                    else if (strcmp(message,"checkright") == 0)
                    {
                        check = checkSpace(player[id].block_x+NEXT_SPACE, player[id].block_y, player[id].rotation,&player[id]);

                        iSendResult = send( read.fd_array[i], &check, sizeof(check), 0 );
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
                            iSendResult = send( read.fd_array[i], (char *)&player[id], sizeof(Player), 0 );
                            if (iSendResult == SOCKET_ERROR)
                            {
                                // showSendError(cid);
                                if (WSAGetLastError() != WSAEWOULDBLOCK)
                                {
                                    //continue;
                                }
                            }
                        }
                        else if (check == FALSE)
                        {

                        }
                    }

                    // check checkdown
                    else if (strcmp(message,"checkdown") == 0)
                    {
                        check = checkNextSpace( &player[id]);

                        iSendResult = send( read.fd_array[i], &check, sizeof(check), 0 );
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
                            iSendResult = send( read.fd_array[i], (char *)&player[id], sizeof(Player), 0 );
                            if (iSendResult == SOCKET_ERROR)
                            {
                                // showSendError(cid);
                                if (WSAGetLastError() != WSAEWOULDBLOCK)
                                {
                                    //continue;
                                }
                            }
                        }
                        else if (check == FALSE)
                        {

                        }
                    }

                    // check checkspace
                    else if (strcmp(message,"checkspace") == 0)
                    {

                        while( (check = checkNextSpace( &player[id])) )
                        {
                            player[id].block_y += 1;
                        }

                        iSendResult = send( read.fd_array[i], (char *)&player[id], sizeof(Player), 0 );
                        if (iSendResult == SOCKET_ERROR)
                        {
                            // showSendError(cid);
                            if (WSAGetLastError() != WSAEWOULDBLOCK)
                            {
                                //continue;
                            }
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

                    //strcpy(message, "hi");
                    //iResult = send(cid, message, sizeof(message),0);
                    /*
                    int len = recv(read.fd_array[i], message, sizeof(message), 0);
                    if(0 == len)
                    {
                        FD_CLR(read.fd_array[i], &read);
                        printf("[%d]님이 접속종료하셧습니다\n", read.fd_array[i]);
                        closesocket(tmp.fd_array[i]);
                    }
                    else
                    {

                        send(read.fd_array[i], message, len, 0);
                    }
                    */
                }
            }
        }
    }
    /*
    // Accept a client socket
    clnt_addr_size=sizeof(clnt_addr);
    ClientSocket = accept(ListenSocket, (struct sockaddr*)&clnt_addr,&clnt_addr_size);
    if (ClientSocket == INVALID_SOCKET) {
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

    // send initial player structure
    initializePlayerSetting(id,&player[id]);
    initializeBlockSetting(&player[id]);
    initializeBoard(&player[id]);
    copyGameBoard( &player[id]);

    iSendResult = send( ClientSocket, (char *)&player[id], sizeof(Player), 0 );
    if (iSendResult == SOCKET_ERROR)
    {
        showSendError(ClientSocket);
    }

    // No longer need server socket
    // closesocket(ListenSocket);

    // Receive until the peer shuts down the connection
    // 이부분에서 서버 작업 처리

    do
    {
        iResult = recv(ClientSocket, message, (int)strlen(message), 0);
        message[iResult] = '\0';
        if (message)
        {
            check = checkNextSpace(&player[id]);
            // printf("%d", checkNextSpace(id,&player[id]));
            iSendResult = send( ClientSocket, &check, sizeof(check), 0 );
            if (iSendResult == SOCKET_ERROR)
            {
                showSendError(ClientSocket);
            }
        }
        // printf("%d", check);
        /*
        iResult = recv(ClientSocket, message, sizeof(message)-1, 0);
        message[iResult] = '\0';
        printf("Message from Client : %s \n", message);
        if (iResult > 0) {
            printf("Server: ");
            gets(message);
            printf("Bytes received: %d\n", iResult);
            iSendResult = send( ClientSocket, message, (int)strlen(message), 0 );
            if (iSendResult == SOCKET_ERROR) {
                showSendError(ClientSocket);
            }
            printf("Bytes Sent: %ld\n", iResult);
        }  else if (iResult == 0) {
            // printf("Connection closing...\n");
            strcpy(message,"");
        }  else  {
            printf("recv failed with error: %d\n", WSAGetLastError());
            closesocket(ClientSocket);
            WSACleanup();
            return 1;
        }
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
    */
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
