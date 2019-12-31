#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS 
#include <stdio.h>
#include <winsock2.h>
#include <locale.h>
#include <windows.h>
#define PORT 666
#pragma  comment  (lib,"Ws2_32.lib")

char* outname(char* inname){
    char* a = inname;
    if ((a = strrchr(inname, '.')) == NULL)
        strcat(a, ".out");
    else
        strcpy(a, ".out");

    return inname;
}

int work(char* file_name) {

    FILE* file = 0;
    if ((file = fopen(file_name, "r")) == NULL) {
        puts("Can't open file");
        return -1;
    }

    FILE* out = 0;
    if ((out = fopen(outname(file_name), "w")) == NULL) {
        puts("Can't create out file");
        return -2;
    }

    printf("<");
    int counter = 0;
    char c = 0;
    while (c != EOF) {
        if (((c = fgetc(file)) < 48)) {
            if ((c == 0x0d) || (c == 0x0a)||(c == EOF))
                break;
            counter++;
            fprintf(out, " ");
         }else{
            fprintf(out, "%c", c);
        }

        printf("%c", c);
    }
   printf(">%d", counter);

    fclose(file);
    fclose(out);

    return counter;
}

int main(int argc, char* argv[]){

   // setlocale(LC_ALL, "Russian");

    char buff[1024];
    printf("UDP Server\n");

    if (WSAStartup(0x202, (WSADATA*)&buff[0])) {
        printf("Error WSAStartup : %d\n", WSAGetLastError());
        return -1;
    }

    SOCKET mysocket;
    if ((mysocket = socket(AF_INET, SOCK_DGRAM, 0)) < 0)   {
        printf("Socket() error: %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    sockaddr_in local_addr;
    local_addr.sin_family = AF_INET;
    local_addr.sin_port = htons(PORT);
    local_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(mysocket, (sockaddr*)&local_addr, sizeof(local_addr)))    {
        printf("bind error: %d\n", WSAGetLastError());
        closesocket(mysocket);
        WSACleanup();
        return -1;
    }

    while (1)    {
        sockaddr_in client_addr;
        int client_addr_size = sizeof(client_addr);
        int bytes_recv = recvfrom(mysocket, &buff[0], sizeof(buff) - 1, 0,
            (sockaddr*)&client_addr, &client_addr_size);
        if (bytes_recv == SOCKET_ERROR)
            printf("recvfrom() error: %d\n", WSAGetLastError());

        HOSTENT* hst;
        hst = gethostbyaddr((char*)&client_addr.sin_addr, 4, AF_INET);
        printf("+%s [%s:%d] new DATAGRAM!\n", (hst) ? hst->h_name : "Unknown host", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        buff[bytes_recv] = 0;
        buff[bytes_recv - 1] = 0;
        printf("TAKE:%s\n", &buff[0]);
        int result;
        if ((result = work(buff)) == -1)
            strcpy(buff, "Cannot open the file."); 
        else    
        if (result == -2)
            strcpy(buff, "Cannot create out file.");
        else
            sprintf(buff, "%d\n", result);

        sendto(mysocket, &buff[0], sizeof(buff), 0, (sockaddr*)&client_addr, sizeof(client_addr));

    }

}