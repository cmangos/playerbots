
#include "playerbot/playerbot.h"
#include "playerbot/PlayerbotAIConfig.h"
#include "playerbot/PlayerbotFactory.h"
#include "PlayerbotCommandServer.h"
#include <cstdlib>
#include <iostream>

INSTANTIATE_SINGLETON_1(PlayerbotCommandServer);

#ifdef CMANGOS

#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>

using boost::asio::ip::tcp;
typedef boost::shared_ptr<tcp::socket> socket_ptr;

bool ReadLine(socket_ptr sock, std::string* buffer, std::string* line)
{
    // Do the real reading from fd until buffer has '\n'.
    std::string::iterator pos;
    while ((pos = find(buffer->begin(), buffer->end(), '\n')) == buffer->end())
    {
        char buf[1025];
        boost::system::error_code error;
        size_t n = sock->read_some(boost::asio::buffer(buf), error);
        if (n == -1 || error == boost::asio::error::eof)
            return false;
        else if (error)
            throw boost::system::system_error(error); // Some other error.

        buf[n] = 0;
        *buffer += buf;
    }

    *line = std::string(buffer->begin(), pos);
    *buffer = std::string(pos + 1, buffer->end());
    return true;
}

void session(socket_ptr sock)
{
    try
    {
        std::string buffer, request;
        while (ReadLine(sock, &buffer, &request)) {
            std::string response = sRandomPlayerbotMgr.HandleRemoteCommand(request) + "\n";
            boost::asio::write(*sock, boost::asio::buffer(response.c_str(), response.size()));
            request = "";
        }
    }
    catch (std::exception& e)
    {
        sLog.outError("%s",e.what());
    }
}

void server(boost::asio::io_context& io_context, short port)
{
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        socket_ptr sock(new tcp::socket(io_context));
        a.accept(*sock);
        boost::thread t(boost::bind(session, sock));
    }
}

void Run()
{
    if (!sPlayerbotAIConfig.commandServerPort) {
        return;
    }

    std::ostringstream s; s << "Starting Playerbot Command Server on port " << sPlayerbotAIConfig.commandServerPort;
    sLog.outString("%s",s.str().c_str());

    try
    {
        boost::asio::io_context io_context;
        server(io_context, sPlayerbotAIConfig.commandServerPort);
    }
    catch (std::exception& e)
    {
        sLog.outError("%s",e.what());
    }
}

void PlayerbotCommandServer::Start()
{
    std::thread serverThread(Run);
    serverThread.detach();
}
#endif

#ifdef MANGOS
bool ReadLine(ACE_SOCK_Stream& client_stream, std::string* buffer, std::string* line)
{
    // Do the real reading from fd until buffer has '\n'.
    std::string::iterator pos;
    while ((pos = find(buffer->begin(), buffer->end(), '\n')) == buffer->end())
    {
        char buf[33];
        size_t n = client_stream.recv_n(buf, 1, 0);
        if (n == -1)
            return false;

        buf[n] = 0;
        *buffer += buf;
    }

    *line = std::string(buffer->begin(), pos);
    *buffer = std::string(pos + 1, buffer->end());
    return true;
}

class PlayerbotCommandServerThread: public ACE_Task <ACE_MT_SYNCH>
{
public:
    int svc(void) {
        if (!sPlayerbotAIConfig.commandServerPort) {
            return 0;
        }

        std::ostringstream s; s << "Starting Playerbot Command Server on port " << sPlayerbotAIConfig.commandServerPort;
        sLog.outString(s.str().c_str());

        ACE_INET_Addr server(sPlayerbotAIConfig.commandServerPort);
        ACE_SOCK_Acceptor client_responder(server);

		while (true)
		{
			ACE_SOCK_Stream client_stream;
			ACE_Time_Value timeout(5);
			ACE_INET_Addr client;
			if (-1 != client_responder.accept(client_stream, &client, &timeout))
			{
				string buffer, request;
				while (ReadLine(client_stream, &buffer, &request))
				{
					string response = sRandomPlayerbotMgr.HandleRemoteCommand(request) + "\n";
					client_stream.send_n(response.c_str(), response.size(), 0);
					request = "";
				}
				client_stream.close();
			}
		}

        return 0;
    }
};


void PlayerbotCommandServer::Start()
{
    PlayerbotCommandServerThread *thread = new PlayerbotCommandServerThread();
    thread->activate();
}

#endif
