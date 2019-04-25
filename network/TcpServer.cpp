#include <cstring>		//memset
#include <netinet/in.h>
#include <network/TcpServer.h>
#include <sys/socket.h>
#include <unistd.h>

#include "network/Select.h"
#include "util.h"

namespace Network
{
	TcpServer::TcpServer(const unsigned short port, const std::string& threadName)
	:	port(port),
	 	serverSocket(0),
	 	run(false),
	 	error(""),
	 	threadName(threadName)
	{
		struct sockaddr_in6 server_addr;

		// create server serverSocket
		serverSocket = socket(AF_INET6, SOCK_STREAM, 0);
		if (serverSocket < 0)
		{
			error = "Unable to create socket for tcp server. Unable to serve clients.";
			return;
		}

		// bind socket to an address (in6addr_any)
		memset((char *) &server_addr, 0, sizeof(server_addr));
		server_addr.sin6_family = AF_INET6;
		server_addr.sin6_addr = in6addr_any;
		server_addr.sin6_port = htons(port);

		int on = 1;
		int intResult = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void*)&on, sizeof(on));
		if (intResult < 0)
		{
			error = "Unable to set tcp server socket option SO_REUSEADDR.";
		}

		intResult = bind(serverSocket, (struct sockaddr *) &server_addr, sizeof(server_addr));
		if (intResult < 0)
		{
			error = "Unable to bind socket for tcp server to port. Unable to serve clients.";
			close(serverSocket);
			return;
		}

		// listen on the serverSocket
		intResult = listen(serverSocket, 5);
		if (intResult != 0)
		{
			error = "Unable to listen on socket for tcp server. Unable to serve clients.";
			close(serverSocket);
			return;
		}

		run = true;

		// create seperate thread that handles the client requests
		serverThread = std::thread(&Network::TcpServer::Worker, this);
	}

	TcpServer::~TcpServer()
	{
		if (run == false)
		{
			return;
		}

		run = false;

		while (connections.size())
		{
			TcpConnection* client = connections.back();
			connections.pop_back();
			delete client;
		}

		serverThread.join();
	}

	void TcpServer::Worker()
	{
		pthread_setname_np(pthread_self(), threadName.c_str());
		fd_set set;
		struct timeval tv;
		struct sockaddr_in6 client_addr;
		socklen_t client_addr_len = sizeof(client_addr);
		while (run == true)
		{
			// wait for connection and abort on shutdown
			int ret;
			do
			{
				FD_ZERO(&set);
				FD_SET(serverSocket, &set);
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &tv));

				if (run == false)
				{
					return;
				}
			} while (ret == 0);

			if (ret < 0)
			{
				continue;
			}

			// accept connection
			int socketClient = accept(serverSocket, (struct sockaddr *) &client_addr, &client_addr_len);
			if (socketClient < 0)
			{
				continue;
			}

			// create client and fill into vector
			auto con = new TcpConnection(socketClient);
			connections.push_back(con);
			Work(con);
		}
	}
}
