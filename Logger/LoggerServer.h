#pragma once

#include <string>
#include <vector>

#include "network/TcpServer.h"

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
			:	Network::TcpServer(port),
			 	run(true)
			{}

			~LoggerServer();
			void Work(Network::TcpConnection* connection) override;

			volatile unsigned char run;
			std::vector<LoggerClient*> clients;
			std::vector<Logger*> loggers;
	};
}
