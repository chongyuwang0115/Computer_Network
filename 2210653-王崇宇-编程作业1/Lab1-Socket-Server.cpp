#include <iostream>
#include <string>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <map>
#include <thread>
#include <chrono>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8000
#define MaxBufSize 1024

using namespace std;

map<SOCKET, string> user_map;
SOCKET sockSrv;
bool isServerRunning = true;

// 获取当前时间字符串
string getCurrentTime() {
	time_t now = time(0);
	struct tm newtime;
	localtime_s(&newtime, &now);  // 使用localtime_s代替localtime
	char timeStr[100];
	strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &newtime);
	return string(timeStr);
}

// 退出处理线程
void ServerInputThread() {
	string input;
	while (isServerRunning) {
		getline(cin, input);
		if (input == "exit" || input == "quit") {
			isServerRunning = false;
			closesocket(sockSrv);
			break;
		}
	}
}

// 处理客户端请求的线程
DWORD WINAPI handlerRequest(LPVOID lparam) {
	SOCKET ClientSocket = (SOCKET)lparam;
	char recvBuf[MaxBufSize];
	char sendBuf[MaxBufSize];

	string userId = to_string(ClientSocket);
	user_map[ClientSocket] = userId;

	send(ClientSocket, userId.c_str(), static_cast<int>(userId.size()), 0);

	// 输出用户连接信息
	cout << "[" << getCurrentTime() << "] [系统日志] 用户[" << userId << "]已连接，Socket ID: " << ClientSocket << endl;

	while (true) {
		int recvResult = recv(ClientSocket, recvBuf, MaxBufSize - 1, 0);
		if (recvResult > 0) {
			recvBuf[recvResult] = '\0';
			cout << "[" << getCurrentTime() << "] [系统日志] 用户[" << userId << "]的消息: " << recvBuf << endl;
			for (const auto& user : user_map) {
				if (user.first != ClientSocket) {
					snprintf(sendBuf, sizeof(sendBuf), "用户[%s]: %s", userId.c_str(), recvBuf);
					send(user.first, sendBuf, static_cast<int>(strlen(sendBuf)), 0);
				}
			}
		}
		else {
			break;
		}
	}

	user_map.erase(ClientSocket);
	cout << "[" << getCurrentTime() << "] [系统日志] 用户[" << userId << "]已退出聊天..." << endl;
	closesocket(ClientSocket);
	return 0;
}

int main() {
	cout << "==================WCY的服务器===================" << endl;
	cout << "WCY的服务器[版本 22.10.65.3]      开发者-Wangchongyu-" << endl;
	cout << "[系统提示] 服务器准备中..." << endl;

	WSAData wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		cout << "[错误] 初始化失败，错误代码: " << WSAGetLastError() << endl;
		return 1;
	}

	sockSrv = socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(PORT);
	inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

	if (bind(sockSrv, (sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
		cout << "[错误] 绑定失败，错误代码: " << WSAGetLastError() << endl;
		WSACleanup();
		return 1;
	}

	listen(sockSrv, SOMAXCONN);
	cout << "[系统提示] 服务器启动成功，等待客户端连接..." << endl;

	cout << "[系统提示] 输入quit或exit关闭服务器" << endl;

	thread inputThread(ServerInputThread);

	while (isServerRunning) {
		SOCKADDR_IN clientAddr;
		int len = sizeof(SOCKADDR_IN);
		SOCKET sockAccept = accept(sockSrv, (SOCKADDR*)&clientAddr, &len);
		if (sockAccept != INVALID_SOCKET) {
			cout << "[" << getCurrentTime() << "] [系统提示] 新用户连接成功!" << endl;
			thread(handlerRequest, (LPVOID)sockAccept).detach(); // 使用线程处理请求
		}
	}

	inputThread.join();
	closesocket(sockSrv);
	WSACleanup();
	return 0;
}