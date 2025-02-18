#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "WHY.h"
//#include "httplib.h"
HMODULE Module = NULL;
SOCKET serverSocket;
HANDLE CT = NULL;

using namespace std;
//#pragma comment (lib, "detours.lib")
//#include <json/json.h>
#include "pch.h"
//INSTALL DETOURS FROM NUGET! (or build from source yourself)
//#include "detours.h"
#include "../Detours/include/detours.h"
#include <unordered_map>
#pragma comment (lib,"../Detours/lib.X64/detours.lib")
//Definitions
typedef BOOL(WINAPI* SetWindowBand)(IN HWND hWnd, IN HWND hwndInsertAfter, IN DWORD dwBand);
typedef BOOL(WINAPI* NtUserEnableIAMAccess)(IN ULONG64 key, IN BOOL enable);

//Fields
NtUserEnableIAMAccess lNtUserEnableIAMAccess;
SetWindowBand lSetWindowBand;

ULONG64 g_iam_key = 0x0;
bool g_is_detached = false; //To prevent detaching twice.
DWORD lSet = 0;
//Forward functions
BOOL WINAPI NtUserEnableIAMAccessHook(ULONG64 key, BOOL enable);
BOOL SetWindowBandInternal(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand);
enum ZBID
{
	ZBID_DEFAULT = 0,
	ZBID_DESKTOP = 1,
	ZBID_UIACCESS = 2,
	ZBID_IMMERSIVE_IHM = 3,
	ZBID_IMMERSIVE_NOTIFICATION = 4,
	ZBID_IMMERSIVE_APPCHROME = 5,
	ZBID_IMMERSIVE_MOGO = 6,
	ZBID_IMMERSIVE_EDGY = 7,
	ZBID_IMMERSIVE_INACTIVEMOBODY = 8,
	ZBID_IMMERSIVE_INACTIVEDOCK = 9,
	ZBID_IMMERSIVE_ACTIVEMOBODY = 10,
	ZBID_IMMERSIVE_ACTIVEDOCK = 11,
	ZBID_IMMERSIVE_BACKGROUND = 12,
	ZBID_IMMERSIVE_SEARCH = 13,
	ZBID_GENUINE_WINDOWS = 14,
	ZBID_IMMERSIVE_RESTRICTED = 15,
	ZBID_SYSTEM_TOOLS = 16,
	//Windows 10+
	ZBID_LOCK = 17,
	ZBID_ABOVELOCK_UX = 18,
};

mutex logMutex;

FILE* loggerfile;

//#include "C:/Users/kill/Downloads/httplib.h"
struct RequestParams {
	HWND hwnd;
	HWND hwnd2;
	DWORD mode;
};
typedef struct _Logdata
{
	const char* level;
	const char* logthing;
	const char* stage;
}_Logdata;
void _WriteLog(LPVOID p) {
	_Logdata* pmd = (_Logdata*)p;
	const char* level = pmd->level;
	const char* logthing = pmd->logthing;
	const char* stage = pmd->stage;

	// 获取当前时间
	time_t dtt = time(0);
	char dt[66];
	ctime_s(dt, sizeof(dt), &dtt);
	dt[strlen(dt) - 1] = '\0';  // 去除 ctime 返回的换行符

	// 使用 ostringstream 拼接日志字符串
	ostringstream oss;
	oss << "[" << level << "][" << stage << ":" << dt << "]:" << logthing << "\n";

	// 使用 ofstream 进行文件写入，并确保线程安全
	lock_guard<mutex> lock(logMutex);  // 锁住日志文件，防止多线程同时写入

	ofstream loggerfile("C:\\explorerhacker.log", ios::app | ios::out | ios::binary);
	if (loggerfile.is_open()) {
		loggerfile << oss.str();  // 写入日志字符串
		loggerfile.close();
	}

	delete pmd;  // 清理动态分配的内存
}
void __WriteLog(const char* level, const char* logthing, const char* stage) {

	// 获取当前时间
	time_t dtt = time(0);
	char dt[66];
	ctime_s(dt, sizeof(dt), &dtt);
	dt[strlen(dt) - 1] = '\0';  // 去除 ctime 返回的换行符

	// 使用 ostringstream 拼接日志字符串
	ostringstream oss;
	oss << "[" << level << "][" << stage << ":" << dt << "]:" << logthing << "\n";

	// 使用 ofstream 进行文件写入，并确保线程安全
	lock_guard<mutex> lock(logMutex);  // 锁住日志文件，防止多线程同时写入

	ofstream loggerfile("C:\\explorerhacker.log", ios::app | ios::out | ios::binary);
	if (loggerfile.is_open()) {
		loggerfile << oss.str();  // 写入日志字符串
		loggerfile.close();
	}
}
void initializeWinsock() {
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		exit(1);
	}
}

SOCKET createListeningSocket(int port) {
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (serverSocket == INVALID_SOCKET) {
		exit(2);
	}

	sockaddr_in serverAddr = {};
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	serverAddr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		int a = GetLastError();
		exit(a);
	}

	if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
		exit(4);
	}

	return serverSocket;
}
// 外部调用的日志写入函数
void WriteLog(const char* level, const char* logthing, const char* stage) {
	_Logdata* n = new _Logdata{
		level,
		logthing,
		stage
	};
	// 创建新线程写入日志
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&_WriteLog, n, NULL, NULL);
}
//Function for detouring NtUserEnableIAMAccess
VOID AttachHook()
{
	const auto path = LoadLibraryW(L"user32.dll");
	__WriteLog("INFO",
		"DLL started,HOOKING...",
		"prepare stage");
	if (path == NULL) {
		DWORD llib = GetLastError();
		__WriteLog("ERROR", "LoadLibraryW user32.dll return" + llib, "prepare stage");
	}
	__WriteLog("DEBUG", "SetWindowBand", "prepare stage");
	lSetWindowBand = SetWindowBand(GetProcAddress(path, "SetWindowBand"));
	if (lSetWindowBand == NULL) {
		DWORD llib = GetLastError();
		__WriteLog("ERROR", "GetProcAddress(SetWindowBand) return" + llib, "prepare stage");
	}
	__WriteLog("DEBUG", "NtUserEnableIAMAccess", "prepare stage");

	lNtUserEnableIAMAccess = NtUserEnableIAMAccess(GetProcAddress(path, MAKEINTRESOURCEA(2510)));
	__WriteLog("DEBUG", "Detour", "prepare stage");

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)lNtUserEnableIAMAccess, (PVOID)NtUserEnableIAMAccessHook);
	long r = DetourTransactionCommit();
	__WriteLog("INFO", "DetourTransactionCommit:" + r, "prepare stage");

}

VOID DetachHook()
{
	if (!g_is_detached)
	{
		DetourRestoreAfterWith();
		DetourTransactionBegin();
		DetourUpdateThread(GetCurrentThread());
		DetourDetach(&(PVOID&)lNtUserEnableIAMAccess, NtUserEnableIAMAccessHook);
		DetourTransactionCommit();
		g_is_detached = true;
	}
}
bool is_valid_hwnd(HWND hwnd) {
	return hwnd != nullptr;
}

bool is_valid_mode(DWORD mode) {
	return mode <= 18;
}

uintptr_t ConvertToUnsignedInt(string input) {
	// 如果字符串以 "0x" 开头，说明是16进制
	if (input.substr(0, 2) == "0x" || input.substr(0, 2) == "0X") {
		return stoull(input, nullptr, 16);  // 以16进制方式转换
	}
	else {
		return stoull(input, nullptr, 10);  // 否则按10进制转换
	}
}


// 处理请求并生成响应
unordered_map<string, string> parseQueryParams(const string& query) {
	unordered_map<string, string> params;
	stringstream ss(query);
	string param;

	while (getline(ss, param, '&')) {
		size_t equalPos = param.find('=');
		if (equalPos != string::npos) {
			string key = param.substr(0, equalPos);
			string value = param.substr(equalPos + 1);
			params[key] = value;
		}
	}

	return params;
}

std::string HandleToHexString(ULONG value) {
	std::stringstream ss;
	ss << "0x" << std::hex << value;
	return ss.str();
}

void cancelTopMost(HWND hwnd) {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	SetWindowPos(hwnd, HWND_NOTOPMOST, rect.left, rect.top, abs(rect.right - rect.left), abs(rect.bottom - rect.top), SWP_SHOWWINDOW);
}
// 处理 HTTP 请求
void handleRequest(SOCKET clientSocket) {
	const int bufferSize = 1024;
	char buffer[bufferSize];

	// 接收客户端请求
	int bytesRead = recv(clientSocket, buffer, bufferSize, 0);
	if (bytesRead <= 0) {
		return;
	}
	buffer[bytesRead] = '\0';  // 确保字符串以 null 结尾

	// 打印接收到的请求
	cout << "Received request:\n" << buffer << endl;

	// 从 HTTP 请求中提取查询参数（只提取 URL 部分）
	string request(buffer);
	for (int i = 0; i < request.size(); i++)
	{
		request[i] = tolower(request[i]);
	}
	size_t methodEnd = request.find(" ");
	size_t urlStart = methodEnd + 1;
	size_t urlEnd = request.find(" ", urlStart);
	string url = request.substr(urlStart, urlEnd - urlStart);
	if (url == "/exit" or url == "/exit/") {
		stringstream response;
		response << "HTTP/1.1 200 OK\r\n";
		response << "Content-Type: text/plain\r\n\r\n";
		response << "SHUTTING DOWN IN 1S!";
		WriteLog("INFO", "SHUTTING DOWN IN 1S!", "HTTP");
		// 发送响应
		send(clientSocket, response.str().c_str(), response.str().length(), 0);
		closesocket(clientSocket);
		Sleep(1000);
		closesocket(serverSocket);
		WSACleanup();
		DetachHook();
		CT = NULL;
		FreeLibraryAndExitThread(Module, 0);
		exit(0);
	}
	else if (g_iam_key) {
		if (url.find("/call") == 0) {
			// 确认 URL 是否有查询参数
			size_t queryStart = url.find('?');
			if (queryStart != string::npos) {
				string query = url.substr(queryStart + 1);  // 提取查询字符串
				// 解析查询参数
				unordered_map<string, string> params = parseQueryParams(query);
				// 获取并转换参数
				string hwndStr = params["hwnd"];
				string hwnd2Str = params["hwnd2"];
				string modeStr = params["mode"];

				uintptr_t hwnd1 = 0;
				uintptr_t hwnd12 = 0;
				uint32_t mode = 2;  // 默认模式为 2

				// 解析 hwnd
				if (!hwndStr.empty()) {
					hwnd1 = ConvertToUnsignedInt(hwndStr);
				}

				// 解析 hwnd2
				if (!hwnd2Str.empty()) {
					hwnd12 = ConvertToUnsignedInt(hwnd2Str);
				}
				// 解析 mode
				if (!modeStr.empty()) {
					mode = static_cast<uint32_t>(ConvertToUnsignedInt(modeStr));
				}
				HWND hwnd = reinterpret_cast<HWND>(hwnd1);
				HWND hwnd2 = reinterpret_cast<HWND>(hwnd12);

				// 校验参数
				if (!is_valid_hwnd(hwnd)) {
					WriteLog("ERROR", "Invalid hwnd", "Validation");
					hwnd = 0;
				}

				if (!is_valid_hwnd(hwnd2)) {
					WriteLog("ERROR", "Invalid hwnd2", "Validation");
					hwnd2 = 0;
				}

				if (!is_valid_mode(mode)) {
					WriteLog("ERROR", "Invalid mode", "Validation");
					mode = 2;
				}
				mode = mode == 0 ? 1 : mode;
				SetWindowBandInternal(hwnd, hwnd2, mode);
				char errorMsg[256];
				FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, lSet, 0, errorMsg, sizeof(errorMsg), NULL);
				// 构建 HTTP 响应
				if (mode == 1) {
					cancelTopMost(hwnd);
				}
				stringstream response;
				response << "HTTP/1.1 200 OK\r\n";
				response << "Content-Type: text/plain\r\n\r\n";
				response << "hwnd=" << hwnd << " hwnd2=" << hwnd2 << " mode=" << mode << " lSet=" << lSet << " " << errorMsg;
				stringstream log;
				log << "hwnd=" << hwnd << " hwnd2=" << hwnd2 << " mode=" << mode << " lSet=" << lSet << " " << errorMsg;

				WriteLog("INFO", log.str().c_str(), "HTTP");
				// 发送响应
				send(clientSocket, response.str().c_str(), response.str().length(), 0);
			}
		}
		else {
			stringstream response;
			response << "HTTP/1.1 404 Not Found\r\n";
			response << "Content-Type: text/plain\r\n\r\n";
			//response << HandleToHexString(g_iam_key).c_str();
			response << "404 Not Found";

			// 发送响应
			send(clientSocket, response.str().c_str(), response.str().length(), 0);
		}
	}
	else {
		stringstream response;
		response << "HTTP/1.1 500 Server Error\r\n";
		response << "Content-Type: text/plain\r\n\r\n";
		response << "ERROR,NEED KEY,WAITING FOR NtUserEnableIAMAccessHook";

		// 发送响应
		send(clientSocket, response.str().c_str(), response.str().length(), 0);
	}

	// 关闭客户端连接
	closesocket(clientSocket);
}

void InitHttpServer() {
	initializeWinsock();

	serverSocket = createListeningSocket(1145);

	while (true) {
		SOCKET clientSocket = accept(serverSocket, NULL, NULL);
		if (clientSocket != INVALID_SOCKET) {
			handleRequest(clientSocket);
		}
	}

	closesocket(serverSocket);
	WSACleanup();
}

bool createthreadflag = false;

//Our detoured function
BOOL WINAPI NtUserEnableIAMAccessHook(ULONG64 key, BOOL enable)
{
	WriteLog("INFO", "AN ILDOT CALL NtUserEnableIAMAccessHook KEY GOT", "NtUserEnableIAMAccessHook");
	const auto res = lNtUserEnableIAMAccess(key, enable);
	if (res == TRUE && !g_iam_key)
	{
		g_iam_key = key;
		DetachHook();
	}
	return res;
}
BOOL SetWindowBandInternal(HWND hWnd, HWND hwndInsertAfter, DWORD dwBand)
{
	if (g_iam_key)
	{
		lNtUserEnableIAMAccess(g_iam_key, TRUE);
		const auto callResult = lSetWindowBand(hWnd, hwndInsertAfter, dwBand);
		lSet = GetLastError();
		lNtUserEnableIAMAccess(g_iam_key, FALSE);
		return callResult;
	}

	return FALSE;
}
bool runned = false;
DWORD dwBytesWrite;
//DllMain
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	//WriteLog("DEBUG","CALL DLLMAIN","PRE");
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	{
		Module = hModule;
		if (!runned) {
			runned = true;

			DisableThreadLibraryCalls(hModule);
			AttachHook();
			__WriteLog("INFO",
				"HOOKED!",
				"prepare stage");
			//CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)&TryForceIAMAccessCallThread, NULL, NULL, NULL);
			if (!createthreadflag) {
				createthreadflag = true;
				__WriteLog("DEBUG", "HTTP THREAD", "NtUserEnableIAMAccessHook");

				CT = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)&InitHttpServer, NULL, 0, NULL);
				__WriteLog("INFO", "HTTP THREAD Created!", "NtUserEnableIAMAccessHook");
			}
		}
	}
	break;
	case DLL_PROCESS_DETACH:
	{
		if (lpReserved != nullptr)
		{
			break; // do not do cleanup if process termination scenario
		}
		__WriteLog("INFO",
			"DLL_THREAD_DETACH",
			"DLL_THREAD_DETACH");
		if (CT != NULL) {
			WaitForSingleObject(CT, 5000); // 等待线程退出
			TerminateThread(CT, 0); // 强制终止线程
			CloseHandle(CT); // 关闭线程句柄
			CT = NULL;
			closesocket(serverSocket);

			WSACleanup(); // 清理 Winsock
			DetachHook();
		}

		return TRUE;
	}
	break;
	}
	return TRUE;
}
