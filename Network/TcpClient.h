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

#pragma once

#include <string>
#include <thread>
#include <vector>

#include "Logger/Logger.h"
#include "Network/TcpConnection.h"

namespace Network
{
	class TcpClient
	{
		public:
			TcpClient() = delete;
			TcpClient(Logger::Logger* logger, const std::string& host, const unsigned short port);
			~TcpClient()
			{
				Terminate();
			}

			void Terminate()
			{
				if (connection == nullptr)
				{
					return;
				}
				delete connection;
				connection = nullptr;
			}

			int Send(const char* buf, const size_t buflen, const int flags = 0)
			{
				if (connection == nullptr)
				{
					errno = ENOTCONN;
					return -1;
				}
				return connection->Send(buf, buflen, flags);
			}

			int Send(const std::string& string, const int flags = 0)
			{
				if (connection == nullptr)
				{
					errno = ENOTCONN;
					return -1;
				}
				return connection->Send(string.c_str(), string.size(), flags);
			}

			int Receive(char* buf, const size_t buflen, const int flags = 0)
			{
				if (connection == nullptr)
				{
					errno = ENOTCONN;
					return -1;
				}
				return connection->Receive(buf, buflen, flags);
			}

			bool IsConnected()
			{
				return connection != nullptr;
			}

		private:
			Logger::Logger* logger;
			const std::string host;
			const unsigned short port;
			TcpConnection* connection;
			bool connected;
	};
}
