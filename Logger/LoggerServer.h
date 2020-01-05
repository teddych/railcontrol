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
#include <vector>

#include "Network/TcpServer.h"

namespace Logger
{
	class Logger;
	class LoggerClient;

	class LoggerServer: private Network::TcpServer
	{
		public:
			LoggerServer(LoggerServer const &) = delete;
			void operator=(LoggerServer const &) = delete;

			Logger* GetLogger(const std::string& component);
			void Send(const std::string& text);

			static const unsigned short defaultPort = 2223;
			static LoggerServer& Instance(const unsigned short port = defaultPort) { static LoggerServer server(port); return server; }

		private:
			LoggerServer(const unsigned short port)
			:	Network::TcpServer(port, "Logger"),
			 	run(true)
			{}

			~LoggerServer();
			void Work(Network::TcpConnection* connection) override;

			volatile unsigned char run;
			std::vector<LoggerClient*> clients;
			std::vector<Logger*> loggers;
	};
}
