/* Rache Handler */

#include <WinSock2.h>
#include <Windows.h>

#include <stdio.h>

#pragma comment(lib, "ws2_32.lib")

#define RACHE_PORT 80
#define BUFFER_SIZE 18384


int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "You did a mistake when you try to execute Rache Handler\n");
        fprintf(stdout, "Usage:\t.\\RacheHandler.exe [IP_ADDR]");

        return 1;
    }

    WSADATA wsaData;

    //LPCSTR fname = "rache.log";
    HANDLE hFile = CreateFileA(".\\rache.log", GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        fprintf(stderr, "Rache.log cannot create\n");
    }

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed, error: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == INVALID_SOCKET) {
        fprintf(stderr, "Socket failed, error: %d\n", WSAGetLastError());
        return 1;
    }

    struct sockaddr_in addr, cli;
    ZeroMemory(&addr, sizeof(addr));
    ZeroMemory(&cli, sizeof(cli));

    addr.sin_addr.s_addr = inet_addr(argv[1]);
    addr.sin_port = htons(RACHE_PORT);
    addr.sin_family = AF_INET;

    if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) == SOCKET_ERROR) {
        fprintf(stderr, "Bind failed, error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }

    if (listen(s, SOMAXCONN) == SOCKET_ERROR) {
        fprintf(stderr, "Listen failed, error: %d\n", WSAGetLastError());
        closesocket(s);
        WSACleanup();
        return 1;
    }

    fprintf(stdout, "RacheHandle running on %s:%d\n", argv[1], RACHE_PORT);

    INT clilen = sizeof(cli);

    while (TRUE) {
        SOCKET ack = accept(s, (struct sockaddr*)&cli, &clilen);
        if (ack == INVALID_SOCKET) {
            fprintf(stderr, "Accept failed, error: %d\n", WSAGetLastError());
            closesocket(s);
            WSACleanup();
            return 1;
        }

        CHAR buffer[BUFFER_SIZE];
        ZeroMemory(buffer, BUFFER_SIZE);

        INT bytesRead = recv(ack, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = 0;
            PCHAR req = strstr(buffer, "\r\n\r\n");

            if (req) {
                if (hFile != INVALID_HANDLE_VALUE) {
                    DWORD bytesWrite = (DWORD)strlen(req);
                    DWORD bytesWritten = 0;
                    BOOL ret = WriteFile(hFile, req, bytesWrite, &bytesWritten, NULL);

                    if (!ret) {
                        fprintf(stderr, "Cannot write into file\n");
                        CloseHandle(hFile);
                    }

                    fprintf(stdout, "Received data: %s\n", req);
                }

                else {
                    fprintf(stdout, "Received Data: %s\n", req);
                }
            }

            else {
                fprintf(stderr, "Request cannot parse, error: %lu\n", GetLastError());
            }

            if (strncmp(buffer, "POST", 4) == 0) {
                const char* response = "HTTP/1.1 200 OK\r\n"
                    "Content-Length: 0\r\n"
                    "Connection: close\r\n"
                    "\r\n";
                send(ack, response, strlen(response), 0);
            }

        }

        else if (bytesRead == 0) {
            fprintf(stderr, "Connection has been closed.\n");
        }

        else {
            fprintf(stderr, "Something going wrong, error :%d\n", WSAGetLastError());
        }

        shutdown(ack, SD_SEND);
        closesocket(ack);

    }

    closesocket(s);
    WSACleanup();

    return 0;
}