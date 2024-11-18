
#include "PlayerbotLLMInterface.h"
#include "PlayerbotAIConfig.h"


#include <iostream>
#include <string>
#include <sstream>

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
    const int bufferSize = 4096;
    char buffer[bufferSize];
    std::string response;

#ifdef _WIN32
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        sLog.outError("BotLLM: WSAStartup failed");
        return "error";
    }
#endif

    // Parse the URL
    ParsedUrl parsedUrl = sPlayerbotAIConfig.llmEndPointUrl;

    // Resolve hostname to IP address
    struct addrinfo hints = {}, * res;
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(parsedUrl.hostname.c_str(), std::to_string(parsedUrl.port).c_str(), &hints, &res) != 0) {
        sLog.outError("BotLLM: Failed to resolve hostname");
#ifdef _WIN32
        WSACleanup();
#endif
        return "error";
    }

    // Create a socket
    int sock;
#ifdef _WIN32
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock == INVALID_SOCKET) {
        sLog.outError("BotLLM: Socket creation failed");
        WSACleanup();
        return "error";
    }
#else
    sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        sLog.outError("BotLLM: Socket creation failed");
        freeaddrinfo(res);
        return "error";
    }
#endif

    // Connect to the server
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        sLog.outError("BotLLM: Connection to server failed");
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        freeaddrinfo(res);
        return "error";
    }

    freeaddrinfo(res); // Free the address info structure

    // Create the HTTP POST request
    std::ostringstream request;
    request << "POST " << parsedUrl.path << " HTTP/1.1\r\n";
    request << "Host: " << parsedUrl.hostname << "\r\n";
    request << "Content-Type: application/json\r\n";
    if (!sPlayerbotAIConfig.llmApiKey.empty())
        request << "Authorization: Bearer " << sPlayerbotAIConfig.llmApiKey;
    std::string body = prompt;
    request << "Content-Length: " << body.size() << "\r\n";
    request << "\r\n";
    request << body;

    // Send the request
    if (send(sock, request.str().c_str(), request.str().size(), 0) < 0) {
        sLog.outError("BotLLM: Failed to send request");
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return "error";
    }

    // Read the response
    int bytesRead;
    while ((bytesRead = recv(sock, buffer, bufferSize - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';
        response += buffer;
    }

#ifdef _WIN32
    if (bytesRead == SOCKET_ERROR) {
        sLog.outError("BotLLM: Error reading response");
    }
    closesocket(sock);
    WSACleanup();
#else
    if (bytesRead < 0) {
        sLog.outError("BotLLM: Error reading response");
    }
    close(sock);
#endif

    // Extract the response body (optional: depending on the server response format)
    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
        response = response.substr(pos + 4);
    }

    return response;
}

std::vector<std::string> PlayerbotLLMInterface::ParseResponse(const std::string& response, std::string startPattern, std::string endPattern)
{
    uint32 startCursor = 0;
    uint32 endCursor = 0;
    std::string subString;

    std::vector<std::string> responses;

    for (auto& c : response)
    {
        if (startCursor < startPattern.size())
        {
            if (c == ' ')
                continue;

            if (c != startPattern[startCursor])
            {
                startCursor = 0;
                continue;
            }

            startCursor++;
            continue;
        }
    
        subString += c;

        if ((subString.size() > 1 && subString.back() == 'n' && subString[subString.size() - 2] == '\\') || (subString.size() > 100 && c == '.') || (subString.size() > 200 && c == ' ') || subString.size() > 250)
        {
            if (subString.back() == 'n' && subString[subString.size() - 2] == '\\')
            {
                subString.pop_back();
                subString.pop_back();
            }
            if(subString.size())
                responses.push_back(subString);
            subString.clear();
        }


        if (c == ' ')
            continue;

        if (c != endPattern[endCursor])
        {
            endCursor = 0;
        }
        else
        {
            if (subString.size() > 1 && subString[subString.size() - 2] == '\\')
                continue; 

            endCursor++;
            if (endCursor == endPattern.size() && responses.size())
            {
                if (subString.size())
                    responses.push_back(subString);

                for (uint32 i = 0; i < std::min(endPattern.size(), responses.back().size()); i++)
                    responses.back().pop_back();
                break;
            }
        }
    }

    return responses;
}