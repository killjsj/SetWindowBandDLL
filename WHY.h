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

