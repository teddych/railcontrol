/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2026 by Teddy / Dominik Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#include <arpa/inet.h>
#include <cstring>		//memset
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "Network/Select.h"
#include "Network/TcpServer.h"
#include "Utils/Network.h"
#include "Utils/Utils.h"

namespace Network
{
	TcpServer::TcpServer(const std::string& address,
		const unsigned short port,
		const std::string& threadName)
	:	run(false),
		wakeupPipe{ -1, -1 },
		error(""),
		address(address),
		port(port),
		threadName(threadName)
	{
	}

	TcpServer::~TcpServer()
	{
		TerminateTcpServer();
	}

	void TcpServer::StartTcpServer()
	{
		if (run)
		{
			return;
		}

		run = true;

		if (pipe(wakeupPipe) != 0)
		{
			wakeupPipe[0] = -1;
			wakeupPipe[1] = -1;
		}
		Utils::Network::SetCloseOnExec(wakeupPipe[0]);
		Utils::Network::SetCloseOnExec(wakeupPipe[1]);

		struct sockaddr_in6 serverAddr6;
		memset(reinterpret_cast<char*>(&serverAddr6), 0, sizeof(serverAddr6));
		serverAddr6.sin6_family = AF_INET6;

		if (address.compare("localhost") == 0)
		{
			serverAddr6.sin6_addr = IN6ADDR_LOOPBACK_INIT;
		}
		else
		{
			serverAddr6.sin6_addr = in6addr_any;
		}
		serverAddr6.sin6_port = htons(port);
		SocketCreateBindListen(serverAddr6.sin6_family, reinterpret_cast<struct sockaddr*>(&serverAddr6));

#ifdef __CYGWIN__
		struct sockaddr_in serverAddr4;
		memset(reinterpret_cast<char*>(&serverAddr4), 0, sizeof(serverAddr4));
		serverAddr4.sin_family = AF_INET;
		if (address.compare("localhost") == 0)
		{
			serverAddr4.sin_addr.s_addr = inet_addr("127.0.0.1");
		}
		else
		{
			serverAddr4.sin_addr.s_addr = htonl(INADDR_ANY);
		}
		serverAddr4.sin_port = htons(port);
		SocketCreateBindListen(serverAddr4.sin_family, reinterpret_cast<struct sockaddr*>(&serverAddr4));
#endif
	}

	void TcpServer::SocketCreateBindListen(int family, struct sockaddr* address)
	{
		int serverSocket = socket(family, SOCK_STREAM, 0);
		if (serverSocket < 0)
		{
			error = "Unable to create socket for tcp server. Unable to serve clients.";
			return;
		}
		Utils::Network::SetCloseOnExec(serverSocket);

		int on = 1;
		int intResult = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void*) &on, sizeof(on));
		if (intResult < 0)
		{
			error = "Unable to set tcp server socket option SO_REUSEADDR.";
			close(serverSocket);
			return;
		}

		intResult = bind(serverSocket, address, family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6));
		if (intResult < 0)
		{
			error = "Unable to bind socket for tcp server to port. Unable to serve clients.";
			close (serverSocket);
			return;
		}

		const int MaxClientsInQueue = 5;
		intResult = listen(serverSocket, MaxClientsInQueue);
		if (intResult != 0)
		{
			error = "Unable to listen on socket for tcp server. Unable to serve clients.";
			close(serverSocket);
			return;
		}

		if (!run)
		{
			close(serverSocket);
			return;
		}
		serverThreads.push_back(std::thread(&Network::TcpServer::Worker, this, serverSocket));
	}

	void TcpServer::TerminateTcpServer()
	{
		run = false;

		// wake up all worker threads waiting in select()
		if (wakeupPipe[1] >= 0)
		{
			const char wakeup = 0;
			__attribute__((unused)) ssize_t unused = write(wakeupPipe[1], &wakeup, sizeof(wakeup));
		}

		while (serverThreads.size())
		{
			std::thread& serverThread = serverThreads.back();
			serverThread.join();
			serverThreads.pop_back();
		}

		for (int& wakeupFd : wakeupPipe)
		{
			if (wakeupFd >= 0)
			{
				close(wakeupFd);
				wakeupFd = -1;
			}
		}
	}

	void TcpServer::Worker(int socket)
	{
		Utils::Utils::SetThreadName(threadName);
		fd_set set;
		struct timeval tv;
		struct sockaddr_storage clientAddress;
		socklen_t clientAddressLength = sizeof(clientAddress);
		const int maxFd = ((wakeupPipe[0] > socket) ? wakeupPipe[0] : socket) + 1;
		while (run == true)
		{
			// wait for connection and abort on shutdown
			int ret;
			do
			{
				FD_ZERO(&set);
				FD_SET(socket, &set);
				if (wakeupPipe[0] >= 0)
				{
					FD_SET(wakeupPipe[0], &set);
				}
				tv.tv_sec = 1;
				tv.tv_usec = 0;
				ret = TEMP_FAILURE_RETRY(select(maxFd, &set, NULL, NULL, &tv));

				if (run == false)
				{
					close(socket);
					return;
				}
			} while (ret == 0);

			if ((ret < 0) || (!FD_ISSET(socket, &set)))
			{
				continue;
			}

			// accept connection
			clientAddressLength = sizeof(clientAddress);
			int socketClient = accept(socket, reinterpret_cast<struct sockaddr*>(&clientAddress), &clientAddressLength);
			if (socketClient < 0)
			{
				continue;
			}
			Utils::Network::SetCloseOnExec(socketClient);

			// create client
			TcpConnection* connection = new TcpConnection(socketClient, &clientAddress);
			Work(connection);
		}
	}
}
