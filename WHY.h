#pragma once
#include "framework.h"
//#include "httplib.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;
#include <mutex>
#include <codecvt>
#include <sstream>
#include <mutex>
#include <ctime>
#include <map>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "../Detours/include/detours.h"
#pragma comment (lib,"detours.lib")
#define InPIPE_NAME L"\\\\.\\pipe\\WindowZBandPipe"
#define null NULL
#include <winsock2.h>
#include <ws2tcpip.h>


#pragma comment(lib, "ws2_32.lib")

void initializeWinsock() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "Winsock initialization failed!" << std::endl;
        exit(1);
    }
}

SOCKET createListeningSocket(int port) {
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed!" << std::endl;
        exit(1);
    }

    sockaddr_in serverAddr = {};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed!" << std::endl;
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed!" << std::endl;
        exit(1);
    }

    return serverSocket;
}