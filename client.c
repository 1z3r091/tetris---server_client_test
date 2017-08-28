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

	char message[30] = {0,};
	int id;
	Player player[PLAYER_NUM];


	if(argc!=3)
	{
		printf("Usage : %s <IP> <port>\n", argv[0]);
		exit(1);
	}

	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
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
	if (iResult == SOCKET_ERROR) {
		ErrorHandling("connect() error!");
    } else {
        // receive id
        iResult = recv(ConnectSocket, &id, (int)sizeof(id), 0);
        if (iResult == SOCKET_ERROR) {
            ErrorHandling("connect() error!");
        }
        printf("Connected to Server! %d\n", ntohl(id));
        iResult = recv(ConnectSocket, (char *)&player[id], sizeof(Player), 0);
        if (iResult == SOCKET_ERROR) {
            ErrorHandling("connect() error!");
        }
        Sleep(3000);
        system("cls");
    }

    // Receive until the peer closes the connection
    // 이 부분에서 클라이언트 작업 처리
    srand(time(NULL));
    setCursorType(NOCURSOR);
    int key_input_frame = 100;
    // id = ONE; // 서버 에서 받아야됨

    showStartMenu();
    drawCompleteBoard(id,&player[id]);
    showCurrentnNextBlock(id,&player[id]);

	do {
        while (TRUE) {
            gameStart(id, ConnectSocket, &player[id]);
        }
        printf("Client: ");
        gets(message);

        if (strcmp(message,"exit") == 0) break;

         // Send an initial buffer
        iResult = send( ConnectSocket, message, (int)strlen(message), 0 );
        if (iResult == SOCKET_ERROR) {
            printf("send failed: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }

        // printf("Bytes Sent: %ld\n", iResult);

        iResult = recv(ConnectSocket, message, sizeof(message)-1, 0);
        if(iResult == -1)
            ErrorHandling("read() error!");
        message[iResult] = '\0';
        printf("Message from server: %s \n", message);
	} while(1);


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
