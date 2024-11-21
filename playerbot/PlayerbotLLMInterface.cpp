
#include "PlayerbotLLMInterface.h"
#include "PlayerbotAIConfig.h"

#include <iostream>
#include <string>
#include <sstream>
#include <regex>

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
        request << "Authorization: Bearer " << sPlayerbotAIConfig.llmApiKey << "\r\n";
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

inline std::string extractAfterPattern(const std::string& content, const std::string& startPattern) {
    std::regex pattern(startPattern);
    std::smatch match;

    if (std::regex_search(content, match, pattern)) {
        size_t start_pos = match.position() + match.length();
        return content.substr(start_pos);
    }
    else {
        return "";
    }

}

inline std::string extractBeforePattern(const std::string& content, const std::string& endPattern) {
    std::regex pattern(endPattern);
    std::smatch match;

    if (std::regex_search(content, match, pattern)) {
        size_t end_pos = match.position();

        return content.substr(0, end_pos);
    }
    else {
        return content;
    }
}

inline std::vector<std::string> splitResponse(const std::string& response, const std::string& splitPattern) {
    std::vector<std::string> result;
    std::regex pattern(splitPattern);
    std::smatch match;
    std::string remaining = response;

    while (std::regex_search(remaining, match, pattern) && remaining.size()) {
        std::string sentence = match[1];

        if (match[2] == "." || match[2] == "!" || match[2] == "?")
            sentence += match[2];

        remaining = match.suffix().str();

        while (sentence.length() > 200) {
            size_t split_pos = sentence.rfind(' ', 200);
            if (split_pos == std::string::npos) {
                split_pos = 200;
            }

            if (!sentence.substr(0, split_pos).empty())
                result.push_back(sentence.substr(0, split_pos));

            sentence = sentence.substr(split_pos + 1);
        }

        if(!sentence.empty())
            result.push_back(sentence);
    }

    return result;
}

std::vector<std::string> PlayerbotLLMInterface::ParseResponse(const std::string& response, const std::string& startPattern, const std::string& endPattern, const std::string& splitPattern)
{
    uint32 startCursor = 0;
    uint32 endCursor = 0;

    std::string actualResponse = response;
    
    actualResponse = extractAfterPattern(actualResponse, startPattern);
    actualResponse = extractBeforePattern(actualResponse, endPattern);

    std::vector<std::string> responses = splitResponse(actualResponse, splitPattern);    

    return responses;
}

void PlayerbotLLMInterface::LimitContext(std::string& context, uint32 currentLength)
{
    if (sPlayerbotAIConfig.llmContextLength && currentLength > sPlayerbotAIConfig.llmContextLength)
    {
        uint32 cutNeeded = currentLength - sPlayerbotAIConfig.llmContextLength;

        if (cutNeeded > context.size())
            context.clear();
        else
        {
            uint32 cutPosition = 0;
            for (auto& c : context)
            {
                cutPosition++;
                if (cutPosition >= cutNeeded && c == ' ' || c == '.')
                    break;
            }
            context = context.substr(cutPosition);
        }
    }
}