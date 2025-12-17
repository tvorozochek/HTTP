#include <iostream>
#include <string>
#include <sstream>

#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {

    setlocale(LC_ALL, "Russian");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
        return 1;
    }

    addrinfo hints{}, * addr = nullptr;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo("127.0.0.1", "8000", &hints, &addr) != 0) {
        cerr << "getaddrinfo failed\n";
        WSACleanup();
        return 1;
    }

    SOCKET listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        WSACleanup();
        return 1;
    }

    if (bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen) == SOCKET_ERROR) {
        cerr << "bind failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(addr);

    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "listen failed: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    cout << "HTTP сервер запущен: http://127.0.0.1:8000\n";

    while (true) {
        SOCKET client = accept(listen_socket, nullptr, nullptr);
        if (client == INVALID_SOCKET) {
            cerr << "accept failed: " << WSAGetLastError() << "\n";
            continue;
        }

        char buffer[1024];
        int bytes = recv(client, buffer, sizeof(buffer) - 1, 0);
        if (bytes <= 0) {
            closesocket(client);
            continue;
        }
        buffer[bytes] = '\0';

        string html =
            "<!DOCTYPE html>"
            "<html>"
            "<head>"
            "<meta charset='utf-8'>"
            "<title> Task 3 </title>"
            "</head>"
            "<body>"
            "<h1>I have a good idea</h1>"
            "<p>But I think you won't read this</p>"
            "<h3>There may be a request from the browser here:</h3>"
            //"<pre>" + std::string(buffer) + "</pre>"
            "</body></html>";

        stringstream response;
        response << "HTTP/1.1 200 OK\r\n"
            << "Content-Type: text/html; charset=utf-8\r\n"
            << "Content-Length: " << html.length() << "\r\n"
            << "Connection: close\r\n"
            << "\r\n"
            << html;

        send(client, response.str().c_str(), (int)response.str().length(), 0);

        closesocket(client);
    }

    closesocket(listen_socket);
    WSACleanup();
    return 0;
}
