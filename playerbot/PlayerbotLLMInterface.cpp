//These methods are supposed to be called asynchronous from the main processes 'and' be greatly delayed by LLM interfernece.
//As such performance (such as opting to use regex) was not a consideration.
//And yes I used chat-gpt to write most of this. LLM for LLM code is what I call fitting.

#include "PlayerbotLLMInterface.h"

#include <iostream>
#include <string>
#include <cstring>
#include <sstream>
#include <regex>
#include <chrono>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <thread>
#include "Log/Log.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netinet/in.h>
#include <cstring>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#endif


namespace
{

    // Win-1251 to Unicode codepoint table for bytes 0x80-0xFF
    static const uint32_t win1251_to_unicode[128] = {
        0x0402,
        0x0403,
        0x201A,
        0x0453,
        0x201E,
        0x2026,
        0x2020,
        0x2021,
        0x20AC,
        0x2030,
        0x0409,
        0x2039,
        0x040A,
        0x040C,
        0x040B,
        0x040F,
        0x0452,
        0x2018,
        0x2019,
        0x201C,
        0x201D,
        0x2022,
        0x2013,
        0x2014,
        0x0000,
        0x2122,
        0x0459,
        0x203A,
        0x045A,
        0x045C,
        0x045B,
        0x045F,
        0x00A0,
        0x040E,
        0x045E,
        0x0408,
        0x00A4,
        0x0490,
        0x00A6,
        0x00A7,
        0x0401,
        0x00A9,
        0x0404,
        0x00AB,
        0x00AC,
        0x00AD,
        0x00AE,
        0x0407,
        0x00B0,
        0x00B1,
        0x0406,
        0x0456,
        0x0491,
        0x00B5,
        0x00B6,
        0x00B7,
        0x0451,
        0x2116,
        0x0454,
        0x00BB,
        0x0458,
        0x0405,
        0x0455,
        0x0457,
        0x0410,
        0x0411,
        0x0412,
        0x0413,
        0x0414,
        0x0415,
        0x0416,
        0x0417,
        0x0418,
        0x0419,
        0x041A,
        0x041B,
        0x041C,
        0x041D,
        0x041E,
        0x041F,
        0x0420,
        0x0421,
        0x0422,
        0x0423,
        0x0424,
        0x0425,
        0x0426,
        0x0427,
        0x0428,
        0x0429,
        0x042A,
        0x042B,
        0x042C,
        0x042D,
        0x042E,
        0x042F,
        0x0430,
        0x0431,
        0x0432,
        0x0433,
        0x0434,
        0x0435,
        0x0436,
        0x0437,
        0x0438,
        0x0439,
        0x043A,
        0x043B,
        0x043C,
        0x043D,
        0x043E,
        0x043F,
        0x0440,
        0x0441,
        0x0442,
        0x0443,
        0x0444,
        0x0445,
        0x0446,
        0x0447,
        0x0448,
        0x0449,
        0x044A,
        0x044B,
        0x044C,
        0x044D,
        0x044E,
        0x044F,
    };

    static void AppendUtf8(std::string& out, uint32_t cp)
    {
        if (cp < 0x80) {
            out += (char)cp;
        }
        else if (cp < 0x800) {
            out += (char)(0xC0 | (cp >> 6));
            out += (char)(0x80 | (cp & 0x3F));
        }
        else {
            out += (char)(0xE0 | (cp >> 12));
            out += (char)(0x80 | ((cp >> 6) & 0x3F));
            out += (char)(0x80 | (cp & 0x3F));
        }
    }

    static bool IsValidUtf8(const std::string& s)
    {
        size_t i = 0;
        while (i < s.size()) {
            unsigned char c = (unsigned char)s[i];
            int bytes;
            if (c < 0x80) {
                i++;
                continue;
            }
            else if ((c & 0xE0) == 0xC0)
                bytes = 2;
            else if ((c & 0xF0) == 0xE0)
                bytes = 3;
            else if ((c & 0xF8) == 0xF0)
                bytes = 4;
            else
                return false;
            for (int j = 1; j < bytes; j++)
                if (i + j >= s.size() || ((unsigned char)s[i + j] & 0xC0) != 0x80)
                    return false;
            i += bytes;
        }
        return true;
    }

    static std::string Win1251ToUtf8(const std::string& input)
    {
        std::string result;
        result.reserve(input.size() * 2);
        for (unsigned char c : input) {
            if (c < 0x80)
                result += (char)c;
            else {
                uint32_t cp = win1251_to_unicode[c - 0x80];
                if (cp)
                    AppendUtf8(result, cp);
            }
        }
        return result;
    }

} // namespace

std::string PlayerbotLLMInterface::SanitizeForJson(const std::string& input)
{
    // If input is not valid UTF-8, assume Win-1251 (WoW 1.12.1 client encoding)
    bool inputIsUtf8 = IsValidUtf8(input);
    const std::string src = inputIsUtf8 ? input : Win1251ToUtf8(input);

    std::string sanitized;
    sanitized.reserve(src.size());
    for (size_t i = 0; i < src.size(); i++) {
        unsigned char c = (unsigned char)src[i];
        switch (c) {
            case '\"': sanitized += "\\\""; break;
            case '\\': sanitized += "\\\\"; break;
            case '\b': sanitized += "\\b"; break;
            case '\f': sanitized += "\\f"; break;
            case '\n': sanitized += "\\n"; break;
            case '\r': sanitized += "\\r"; break;
            case '\t': sanitized += "\\t"; break;
            default:
                if (c < 0x20) {
                    char buffer[7];
                    snprintf(buffer, sizeof(buffer), "\\u%04x", c);
                    sanitized += buffer;
                }
                else {
                    sanitized += (char)c;
                }
        }
    }
    return sanitized;
}

inline void SetNonBlockingSocket(int sock) {
#ifdef _WIN32
    u_long mode = 1;
    if (ioctlsocket(sock, FIONBIO, &mode) != 0) {
        sLog.outError("BotLLM: Failed to set non-blocking mode on socket. Error: %d", WSAGetLastError());
    }
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags == -1 || fcntl(sock, F_SETFL, flags | O_NONBLOCK) == -1) {
        sLog.outError("BotLLM: Failed to set non-blocking mode on socket. Error: %s", strerror(errno));
    }
#endif
}

inline void RestoreBlockingSocket(int sock) {
#ifdef _WIN32
    u_long mode = 0;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    int flags = fcntl(sock, F_GETFL, 0);
    if (flags != -1) {
        fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
    }
#endif
}

inline std::string SSLRecvWithTimeout(SSL* ssl, int timeout_seconds, int& bytesRead) {
    char buffer[4096];
    std::string response;
    
    auto start = std::chrono::steady_clock::now();
    
    while (true) {
        bytesRead = SSL_read(ssl, buffer, sizeof(buffer) - 1);
        
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }
        else {
            int ssl_error = SSL_get_error(ssl, bytesRead);
            if (ssl_error == SSL_ERROR_WANT_READ || ssl_error == SSL_ERROR_WANT_WRITE) {
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= timeout_seconds) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            else {
                break;
            }
        }
    }
    
    return response;
}

inline std::string RecvWithTimeout(int sock, int timeout_seconds, int& bytesRead) {
    char buffer[4096];
    int bufferSize = sizeof(buffer);
    std::string response;

    SetNonBlockingSocket(sock);

    auto start = std::chrono::steady_clock::now();

    while (true) {
        bytesRead = recv(sock, buffer, bufferSize - 1, 0);

        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            response += buffer;
        }
        else if (bytesRead == -1) {
#ifdef _WIN32
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
#else
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
#endif
                auto now = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::seconds>(now - start).count() >= timeout_seconds) {
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            else {
#ifdef _WIN32
                sLog.outError("BotLLM: recv error: %d", WSAGetLastError());
#else
                sLog.outError("BotLLM: recv error: %s", strerror(errno));
#endif
                break;
            }
            }
        else {
            break;
        }
        }

    RestoreBlockingSocket(sock);

    return response;
    }

std::string GetSSLError() {
    unsigned long err = ERR_get_error();
    char err_buf[256];
    ERR_error_string_n(err, err_buf, sizeof(err_buf));
    return std::string(err_buf);
}

std::string PlayerbotLLMInterface::Generate(const std::string& prompt, int timeOutSeconds, int maxGenerations, std::vector<std::string> & debugLines) {
    bool debug = !debugLines.empty();

    if (sPlayerbotLLMInterface.generationCount > maxGenerations)
    {
        if (debug)
            debugLines.push_back("Maximum generations reached " + std::to_string(sPlayerbotLLMInterface.generationCount) + "/" + std::to_string(maxGenerations));
        return {};
    }

    sPlayerbotLLMInterface.generationCount++;

    if (debug)
        debugLines.push_back("Generations start " + std::to_string(sPlayerbotLLMInterface.generationCount) + "/" + std::to_string(maxGenerations));

#ifdef _WIN32
    if (debug)
        debugLines.push_back("Initialize Winsock");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        if (debug)
            debugLines.push_back("WSAStartup failed");

        sLog.outError("BotLLM: WSAStartup failed");
        sPlayerbotLLMInterface.generationCount--;
        return "error";
    }
#endif

    ParsedUrl parsedUrl = sPlayerbotAIConfig.llmEndPointUrl;

    if (debug)
        debugLines.push_back("Resolve hostname to IP address: " + parsedUrl.hostname + " " + std::to_string(parsedUrl.port));

    struct addrinfo hints = {}, * res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    
    std::string portStr = std::to_string(parsedUrl.port);
    if (getaddrinfo(parsedUrl.hostname.c_str(), portStr.c_str(), &hints, &res) != 0) {
        if (debug)
            debugLines.push_back("Failed to resolve hostname: " + parsedUrl.hostname);

        sLog.outError("BotLLM: Failed to resolve hostname: %s", parsedUrl.hostname.c_str());
#ifdef _WIN32
        WSACleanup();
#endif
        sPlayerbotLLMInterface.generationCount--;
        return "error";
    }

    if (debug)
        debugLines.push_back("Create a socket");
    
    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    
#ifdef _WIN32
    if (sock == INVALID_SOCKET) {
        if (debug)
            debugLines.push_back("Socket creation failed");

        sLog.outError("BotLLM: Socket creation failed. Error: %d", WSAGetLastError());
        WSACleanup();
        freeaddrinfo(res);
        sPlayerbotLLMInterface.generationCount--;
        return "error";
    }
#else
    if (sock < 0) {
        if (debug)
            debugLines.push_back("Socket creation failed");

        sLog.outError("BotLLM: Socket creation failed. Error: %s", strerror(errno));
        freeaddrinfo(res);
        sPlayerbotLLMInterface.generationCount--;
        return "error";
    }
#endif

    if (debug)
        debugLines.push_back("Connect to the server");

    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        if (debug)
            debugLines.push_back("Connection to server failed");

#ifdef _WIN32
        sLog.outError("BotLLM: Connection to server failed. Error: %d", WSAGetLastError());
        closesocket(sock);
        WSACleanup();
#else
        sLog.outError("BotLLM: Connection to server failed. Error: %s", strerror(errno));
        close(sock);
#endif
        freeaddrinfo(res);
        sPlayerbotLLMInterface.generationCount--;
        return "error";
    }

    freeaddrinfo(res);

    SSL_CTX* ctx = nullptr;
    SSL* ssl = nullptr;
    std::string response;

    if (parsedUrl.https)
    {
        static bool ssl_initialized = false;
        if (!ssl_initialized) {
            SSL_library_init();
            SSL_load_error_strings();
            OpenSSL_add_all_algorithms();
            ssl_initialized = true;
        }

        const SSL_METHOD* method = TLS_client_method();
        ctx = SSL_CTX_new(method);
        if (!ctx) {
            if (debug)
                debugLines.push_back("Failed to create SSL context: " + GetSSLError());
            sLog.outError("BotLLM: Failed to create SSL context: %s", GetSSLError().c_str());
#ifdef _WIN32
            closesocket(sock);
            WSACleanup();
#else
            close(sock);
#endif
            sPlayerbotLLMInterface.generationCount--;
            return "error";
        }

        SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
        SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

        ssl = SSL_new(ctx);
        if (!ssl) {
            if (debug)
                debugLines.push_back("Failed to create SSL object: " + GetSSLError());
            sLog.outError("BotLLM: Failed to create SSL object: %s", GetSSLError().c_str());
            SSL_CTX_free(ctx);
#ifdef _WIN32
            closesocket(sock);
            WSACleanup();
#else
            close(sock);
#endif
            sPlayerbotLLMInterface.generationCount--;
            return "error";
        }

        SSL_set_tlsext_host_name(ssl, parsedUrl.hostname.c_str());
        
        SSL_set_fd(ssl, sock);
        
        if (debug)
            debugLines.push_back("Performing SSL handshake...");
            
        int ssl_connect_result = SSL_connect(ssl);
        if (ssl_connect_result <= 0) {
            int ssl_error = SSL_get_error(ssl, ssl_connect_result);
            std::string error_msg = "SSL connection failed. SSL Error: " + std::to_string(ssl_error) + " - " + GetSSLError();
            
            if (debug)
                debugLines.push_back(error_msg);
            sLog.outError("BotLLM: %s", error_msg.c_str());
            
            SSL_free(ssl);
            SSL_CTX_free(ctx);
#ifdef _WIN32
            closesocket(sock);
            WSACleanup();
#else
            close(sock);
#endif
            sPlayerbotLLMInterface.generationCount--;
            return "error";
        }
        
        if (debug)
            debugLines.push_back("SSL handshake successful");
    }

    std::ostringstream request;
    request << "POST " << parsedUrl.path << " HTTP/1.1\r\n";
    request << "Host: " << parsedUrl.hostname << "\r\n";
    request << "Content-Type: application/json\r\n";
    request << "Accept: application/json\r\n";
    request << "Connection: close\r\n";
    
    if (!sPlayerbotAIConfig.llmApiKey.empty())
        request << "Authorization: Bearer " << sPlayerbotAIConfig.llmApiKey << "\r\n";
    
    std::string body = prompt;
    request << "Content-Length: " << body.size() << "\r\n";
    request << "\r\n";
    request << body;

    std::string requestStr = request.str();
    
    if (debug)
        debugLines.push_back("Send the request: " + requestStr);

    int write_result;
    if (parsedUrl.https && ssl) {
        write_result = SSL_write(ssl, requestStr.c_str(), requestStr.size());
        if (write_result <= 0) {
            int ssl_error = SSL_get_error(ssl, write_result);
            std::string error_msg = "Failed to send SSL request. SSL Error: " + std::to_string(ssl_error) + " - " + GetSSLError();
            if (debug)
                debugLines.push_back(error_msg);
            sLog.outError("BotLLM: %s", error_msg.c_str());
        }
    } else {
        write_result = send(sock, requestStr.c_str(), requestStr.size(), 0);
        if (write_result < 0) {
            if (debug)
                debugLines.push_back("Failed to send request");
            sLog.outError("BotLLM: Failed to send request");
        }
    }

    if ((parsedUrl.https && write_result <= 0) || (!parsedUrl.https && write_result < 0)) {
        if (parsedUrl.https && ssl) {
            SSL_free(ssl);
            SSL_CTX_free(ctx);
        }
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        sPlayerbotLLMInterface.generationCount--;
        return "error";
    }

    if (debug)
        debugLines.push_back("Read the response");

    int bytesRead = 0;
    
    if (parsedUrl.https && ssl) {
        response = SSLRecvWithTimeout(ssl, timeOutSeconds, bytesRead);
    } else {
        response = RecvWithTimeout(sock, timeOutSeconds, bytesRead);
    }

    if (parsedUrl.https && ssl) {
        SSL_shutdown(ssl);
        SSL_free(ssl);
        SSL_CTX_free(ctx);
    }

#ifdef _WIN32
    if (bytesRead == SOCKET_ERROR) {
        if (debug)
            debugLines.push_back("Error reading response");
        sLog.outError("BotLLM: Error reading response. Error: %d", WSAGetLastError());
    }
    closesocket(sock);
    WSACleanup();
#else
    if (bytesRead < 0) {
        if (debug)
            debugLines.push_back("Error reading response");
        sLog.outError("BotLLM: Error reading response. Error: %s", strerror(errno));
    }
    close(sock);
#endif

    sPlayerbotLLMInterface.generationCount--;

    if (debug)
    {
        if (!response.empty())
            debugLines.push_back(response);
        else
            debugLines.push_back("Empty response");
    }

    size_t pos = response.find("\r\n\r\n");
    if (pos != std::string::npos) {
        response = response.substr(pos + 4);
        if (debug)
            debugLines.push_back("HTTP Body: " + response);
    }

    return response;
}

inline std::string extractAfterPattern(const std::string& content, const std::string& startPattern) {
    if (startPattern.empty())
        return content;
        
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
    if (endPattern.empty())
        return content;
        
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
    
    if (splitPattern.empty()) {
        result.push_back(response);
        return result;
    }
    
    std::regex pattern(splitPattern);
    std::smatch match;
    
    std::sregex_iterator begin(response.begin(), response.end(), pattern);
    std::sregex_iterator end;
    for (auto it = begin; it != end; ++it) {
        result.push_back(it->str());
    }

    if(result.empty())
        result.push_back(response);

    return result;
}

std::vector<std::string> PlayerbotLLMInterface::ParseResponse(const std::string& response, const std::string& startPattern, const std::string& endPattern, const std::string& deletePattern, const std::string& splitPattern, std::vector<std::string>& debugLines)
{
    bool debug = !(debugLines.empty());
    uint32 startCursor = 0;
    uint32 endCursor = 0;

    std::string actualResponse = response;

    if (debug)
        debugLines.push_back("start pattern:" + startPattern);
    
    actualResponse = extractAfterPattern(actualResponse, startPattern);

    PlayerbotTextMgr::ReplaceAll(actualResponse, R"(\")", "'");

    if (debug)
    {
        debugLines.push_back(!actualResponse.empty() ? actualResponse : "Empty response");
        debugLines.push_back("end pattern:" + endPattern);
    }

    actualResponse = extractBeforePattern(actualResponse, endPattern);

    if (debug)
    {
        debugLines.push_back(!actualResponse.empty() ? actualResponse : "Empty response");
        debugLines.push_back("delete pattern:" + deletePattern);
    }

    if (!deletePattern.empty()) {
        std::regex regexPattern(deletePattern);
        actualResponse = std::regex_replace(actualResponse, regexPattern, "");
    }

    if (debug)
    {
        debugLines.push_back(!actualResponse.empty() ? actualResponse : "Empty response");
        debugLines.push_back("split pattern:" + splitPattern);
    }

    std::vector<std::string> responses = splitResponse(actualResponse, splitPattern);   

    if (debug)
        debugLines.insert(debugLines.end(), responses.begin(), responses.end());

    return responses;
}

void PlayerbotLLMInterface::LimitContext(std::string& context, int currentLength)
{
    if (sPlayerbotAIConfig.llmContextLength && (uint32)currentLength > sPlayerbotAIConfig.llmContextLength)
    {
        uint32 cutNeeded = currentLength - sPlayerbotAIConfig.llmContextLength;

        if (cutNeeded >= context.size())
        {
            context.clear();
        }
        else
        {
            uint32 cutPosition = cutNeeded;
            for (size_t i = cutNeeded; i < context.size(); ++i) {
                if (context[i] == ' ' || context[i] == '.') {
                    cutPosition = i + 1;
                    break;
                }
            }
            
            if (cutPosition < context.size()) {
                context = context.substr(cutPosition);
            }
            else {
                context.clear();
            }
        }
    }
}
