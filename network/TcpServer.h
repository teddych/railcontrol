#pragma once

#include <thread>
#include <vector>

#include "TcpConnection.h"

namespace Network
{
	class TcpServer
	{
		public:
			TcpServer(const unsigned short port);
			~TcpServer();

			virtual void Work(Network::TcpConnection* connection) = 0;

		private:
			void Worker();

			unsigned short port;
			int serverSocket;
			volatile bool run;
			std::thread serverThread;
			std::vector<TcpConnection*> connections;
	};
}
