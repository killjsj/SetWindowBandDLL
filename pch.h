// pch.h: 这是预编译标头文件。
// 下方列出的文件仅编译一次，提高了将来生成的生成性能。
// 这还将影响 IntelliSense 性能，包括代码完成和许多代码浏览功能。
// 但是，如果此处列出的文件中的任何一个在生成之间有更新，它们全部都将被重新编译。
// 请勿在此处添加要频繁更新的文件，这将使得性能优势无效。

#ifndef PCH_H
#define PCH_H
// 添加要在此处预编译的标头
#include "framework.h"
//#include "httplib.h"

#define _WINSOCK_DEPRECATED_NO_WARNINGS
using namespace std;
#include <mutex>
#include <codecvt>
#include <sstream>
#include <ctime>
#include <map>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "../Detours/include/detours.h"
#pragma comment (lib,"detours.lib")
#define InPIPE_NAME L"\\\\.\\pipe\\WindowZBandPipe"
#define null NULL
#endif //PCH_H
