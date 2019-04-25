#pragma once

#include <string>
#include <thread>
#include <vector>

#include "network/TcpConnection.h"

namespace Network
{
	class TcpServer
	{
		public:
			TcpServer(const unsigned short port, const std::string& threadName);
			~TcpServer();

			virtual void Work(Network::TcpConnection* connection) = 0;

		private:
			void Worker();

			unsigned short port;
			int serverSocket;
			volatile bool run;
			std::thread serverThread;
			std::vector<TcpConnection*> connections;
			std::string error;
			const std::string threadName;
	};
}
