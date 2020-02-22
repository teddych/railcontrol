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
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "Network/TcpClient.h"

namespace Network
{
	TcpClient::TcpClient(Logger::Logger* logger, const std::string& host, const unsigned short port)
	:	logger(logger),
	 	host(host),
	 	port(port),
	 	connection(nullptr),
	 	connected(false)
	{
	    struct sockaddr_in ecosAddress;
	    ecosAddress.sin_family = AF_INET;
	    ecosAddress.sin_port = htons(port);
	    int ok = inet_pton(AF_INET, host.c_str(), &ecosAddress.sin_addr);
	    if (ok <= 0)
	    {
	        printf("\nInvalid address/ Address not supported \n");
	        return;
	    }

	    int sock = socket(AF_INET, SOCK_STREAM, 0);
	    if (sock < 0)
	    {
	        printf("\n Socket creation error \n");
	        return;
	    }

	    ok = connect(sock, (struct sockaddr *)&ecosAddress, sizeof(ecosAddress));
	    if (ok < 0)
	    {
	        printf("\nConnection Failed \n");
	        close(sock);
	        return;
	    }

		connection = new TcpConnection(sock);
	}
}
