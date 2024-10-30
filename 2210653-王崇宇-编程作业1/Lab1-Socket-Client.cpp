#include <iostream>
#include <string>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <thread>
#include <chrono>
#include <ctime>

#pragma comment(lib, "ws2_32.lib")

#define PORT 8000
#define MaxBufSize 1024

using namespace std;

SOCKET clientSocket;
string user_id;
bool isClientRunning = true;

// 获取当前时间字符串
string getCurrentTime() {
    time_t now = time(0);
    struct tm newtime;
    localtime_s(&newtime, &now);  // 使用localtime_s代替localtime
    char timeStr[100];
    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &newtime);
    return string(timeStr);
}

// 接收消息的线程
DWORD WINAPI recvThread(LPVOID lparam) {
    SOCKET ClientSocket = (SOCKET)lparam;
    char recvBuf[MaxBufSize];

    while (isClientRunning) {
        int recvResult = recv(ClientSocket, recvBuf, MaxBufSize - 1, 0);
        if (recvResult > 0) {
            recvBuf[recvResult] = '\0';
            cout << "[" << getCurrentTime() << "] 收到消息: " << recvBuf << endl;
            cout << "用户[" << user_id << "]> " << std::flush; // 输出提示符并立即刷新输出缓冲区
        }
        else {
            cout << "[警告] 服务器断开连接..." << endl;
            isClientRunning = false;
            break;
        }
    }
    return 0;
}

int main() {
    cout << "==================WCY的客户端===================" << endl;
    cout << "WCY的客户端[版本 22.10.65.3]      开发者-Wangchongyu-" << endl;

    WSAData wsadata;
    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        cout << "[错误] 初始化Socket失败，错误代码: " << WSAGetLastError() << endl;
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        cout << "[错误] 创建Socket失败，错误代码: " << WSAGetLastError() << endl;
        WSACleanup();
        return 1;
    }

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr);

    if (connect(clientSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        cout << "[错误] 服务器连接失败，错误代码: " << WSAGetLastError() << endl;
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    char recvBuf[MaxBufSize];
    int recvResult = recv(clientSocket, recvBuf, MaxBufSize - 1, 0);
    if (recvResult > 0) {
        recvBuf[recvResult] = '\0';
        user_id = recvBuf;
        cout << "[" << getCurrentTime() << "] [系统提示] 已连接服务器，分配的用户ID为: " << user_id << endl;
    }

    CreateThread(NULL, 0, recvThread, (LPVOID)clientSocket, 0, NULL); // 创建接收消息线程

    // 添加提示信息
    cout << "[系统提示] 输入quit或exit退出聊天" << endl;

    while (isClientRunning) {
        cout << "用户[" << user_id << "]> ";
        string message;
        getline(cin, message);
        if (message == "exit" || message == "quit") {
            isClientRunning = false;
            break;
        }

        // 在发送消息时显示当前时间
        string currentTime = getCurrentTime();
        cout << "[" << currentTime << "] 发送消息: " << message << endl;

        send(clientSocket, message.c_str(), static_cast<int>(message.size()), 0);
    }

    closesocket(clientSocket);
    WSACleanup();
    return 0;
}
