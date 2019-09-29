#pragma once

#include <string>

#include "Network/TcpServer.h"

namespace Logger
{
	class LoggerClient
	{
		public:
			LoggerClient(Network::TcpConnection* connection)
			:	connection(connection)
			{}

			~LoggerClient()
			{
				connection->Terminate();
			}

			void Send(const std::string& s) { connection->Send(s); }

		private:
			Network::TcpConnection* connection;
	};
}
