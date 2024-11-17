
#include "PlayerbotLLMInterface.h"
#include <iostream>
#include <string>
#include <sstream>

#ifdef _WIN32
// Windows-specific headers
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib") // Link with Ws2_32.lib
#else
// Linux/Unix-specific headers
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#endif

std::string PlayerbotLLMInterface::Generate(const std::string& prompt) {
    std::string serverIp = "127.0.0.1"; // Replace with KoboldCpp server IP
    int serverPort = 5001;             // Replace with KoboldCpp server port

    const int bufferSize = 4096;
    char buffer[bufferSize];
    std::string response;

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        return "WSAStartup failed";
    }
#endif

    // Create a socket
    int sock;
#ifdef _WIN32
    sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock == INVALID_SOCKET) {
        WSACleanup();
        return "Socket creation failed";
    }
#else
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        return "Socket creation failed";
    }
#endif

    // Server address setup
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(serverPort);
#ifdef _WIN32
    if (InetPton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
        closesocket(sock);
        WSACleanup();
        return "Invalid server IP address";
    }
#else
    if (inet_pton(AF_INET, serverIp.c_str(), &serverAddr.sin_addr) <= 0) {
        close(sock);
        return "Invalid server IP address";
    }
#endif

    // Connect to the server
#ifdef _WIN32
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return "Connection to server failed";
    }
#else
    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        close(sock);
        return "Connection to server failed";
    }
#endif

    // Create the HTTP POST request
    std::ostringstream request;
    request << "POST /api/v1/generate HTTP/1.1\r\n";
    request << "Host: " << serverIp << ":" << serverPort << "\r\n";
    request << "Content-Type: application/json\r\n";
    std::string body = "{\"prompt\": \"" + prompt + "\"}";
    request << "Content-Length: " << body.size() << "\r\n";
    request << "\r\n";
    request << body;

    // Send the request
#ifdef _WIN32
    if (send(sock, request.str().c_str(), request.str().size(), 0) == SOCKET_ERROR) {
        closesocket(sock);
        WSACleanup();
        return "Failed to send request";
    }
#else
    if (send(sock, request.str().c_str(), request.str().size(), 0) < 0) {
        close(sock);
        return "Failed to send request";
    }
#endif

    // Read the response
    int bytesRead;
    while ((bytesRead = recv(sock, buffer, bufferSize - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }

#ifdef _WIN32
    if (bytesRead == SOCKET_ERROR) {
        response += "Error reading response";
    }
    closesocket(sock);
    WSACleanup();
#else
    if (bytesRead < 0) {
        response += "Error reading response";
    }
    close(sock);
#endif

    // Extract the response body (optional: depending on KoboldCpp response format)
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
        response = response.substr(pos + 4);
    }

    return response;
}