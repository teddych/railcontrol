/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2020 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include <cstring>		//memset
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <iostream>

#include "Network/Select.h"
#include "Network/TcpServer.h"
#include "Utils/Utils.h"

namespace Network
{
	TcpServer::TcpServer(const unsigned short port, const std::string& threadName)
	:	run(false),
	 	error(""),
	 	threadName(threadName)
	{
		struct addrinfo hints;
		memset(&hints, 0, sizeof(hints));
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_flags = AI_PASSIVE;
		hints.ai_protocol = IPPROTO_TCP;
		hints.ai_canonname = nullptr;
		hints.ai_addr = nullptr;
		hints.ai_next = nullptr;

		struct addrinfo* addrInfos;
		int intResult = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &addrInfos);
		if (intResult != 0)
		{
			error = "Unable to get addresses. Unable to serve clients.";
			return;
		}

		char buffer[sizeof(struct in6_addr)];
		for (struct addrinfo* addrInfo = addrInfos; addrInfo != nullptr; addrInfo = addrInfo->ai_next)
		{
		    struct in_addr* addr = nullptr;
		    struct sockaddr* ip = nullptr;
		    if (addrInfo->ai_family == AF_INET){
		        struct sockaddr_in* ipv4 = reinterpret_cast<struct sockaddr_in*>(addrInfo->ai_addr);
		        ip = reinterpret_cast<struct sockaddr*>(ipv4);
		        addr = &(ipv4->sin_addr);
		    }
		    else if (addrInfo->ai_family == AF_INET6)
		    {
		        struct sockaddr_in6* ipv6 = reinterpret_cast<struct sockaddr_in6*>(addrInfo->ai_addr);
		        ip = reinterpret_cast<struct sockaddr*>(ipv6);
		        addr = reinterpret_cast<struct in_addr*>(&(ipv6->sin6_addr));
		    }
		    else
		    {
		    	continue;
		    }

		 	const char* charPResult = inet_ntop(addrInfo->ai_family, addr, buffer, sizeof(buffer));
			if (charPResult != nullptr)
			{
				std::cout
					<< "ai_family: " << addrInfo->ai_family
					<< " ai_socktype: " << addrInfo->ai_socktype
					<< " ai_protocol: " << addrInfo->ai_protocol
					<< " ai_address:" << buffer
					<< std::endl;
			}
			SocketCreateBindListen(addrInfo->ai_family, ip);
		}

		freeaddrinfo(addrInfos);
	}

	TcpServer::~TcpServer()
	{
		TerminateTcpServer();

		while (connections.size())
		{
			TcpConnection* client = connections.back();
			connections.pop_back();
			delete client;
		}
	}

	void TcpServer::SocketCreateBindListen(int family, struct sockaddr* address)
	{
		int serverSocket = socket(family, SOCK_STREAM, 0);
		if (serverSocket < 0)
		{
			error = "Unable to create socket for tcp server. Unable to serve clients.";
			return;
		}

		int on = 1;
		int intResult = setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const void*) &on, sizeof(on));
		if (intResult < 0)
		{
			error = "Unable to set tcp server socket option SO_REUSEADDR.";
			close(serverSocket);
			return;
		}

		intResult = bind(serverSocket, address, sizeof(struct sockaddr));
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

		run = true;
		serverThreads.push_back(std::thread(&Network::TcpServer::Worker, this, serverSocket));
	}

	void TcpServer::TerminateTcpServer()
	{
		if (run == false)
		{
			return;
		}

		run = false;

		for(std::thread& serverThread : serverThreads)
		{
			serverThread.join();
		}
	}

	void TcpServer::Worker(int socket)
	{
		Utils::Utils::SetThreadName(threadName);
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
				FD_SET(socket, &set);
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
			int socketClient = accept(socket, reinterpret_cast<struct sockaddr*>(&client_addr), &client_addr_len);
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
