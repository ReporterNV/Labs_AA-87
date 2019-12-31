#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 1
#include <winsock2.h>
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <locale.h>
#pragma  comment  (lib,"Ws2_32.lib")

#define PORT 666
#define SERVERADDR "127.0.0.1"

int main(int argc, char* argv[])
{
    char buff[10 * 1014];
    printf("UDP DEMO Client\nType quit to quit\n\n");

    if (WSAStartup(0x202, (WSADATA*)&buff[0])) {
        printf("WSAStartup error: %d\n", WSAGetLastError());
        return -1;
    }

    SOCKET my_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (my_sock == INVALID_SOCKET){
        printf("socket() error: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    HOSTENT* hst;
    sockaddr_in dest_addr;

    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = htons(PORT);

    if (inet_addr(SERVERADDR) != INADDR_NONE){
        dest_addr.sin_addr.s_addr = inet_addr(SERVERADDR);
    } else  {
        if (hst = gethostbyname(SERVERADDR)){
            dest_addr.sin_addr.s_addr = ((unsigned long**)hst->h_addr_list)[0][0];
        } else {
            printf("Unknown host: %d\n", WSAGetLastError());
            closesocket(my_sock);
            WSACleanup();
            return -1;
        }
    }

    while (1)  {
        printf("\nCMD>");

        fgets(&buff[0], sizeof(buff) - 1, stdin);

        if (!strcmp(&buff[0], "exit\n")) break;

        sendto(my_sock, &buff[0], strlen(&buff[0]), 0, (sockaddr*)&dest_addr, sizeof(dest_addr));

        sockaddr_in server_addr;
        int server_addr_size = sizeof(server_addr);

        int n = recvfrom(my_sock, &buff[0], sizeof(buff) - 1, 0, (sockaddr*)&server_addr, &server_addr_size);

        if (n == SOCKET_ERROR) {
            printf("recvfrom() error: %d\n", WSAGetLastError());
            closesocket(my_sock);
            WSACleanup();
            return -1;
        }

        buff[n] = 0;

        printf("Take>%s", &buff[0]);
    }


    closesocket(my_sock);
    WSACleanup();
    return 0;
}