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

			Logger& GetLogger(const std::string& component);
			void Send(const std::string& text);

			static LoggerServer& Instance() { static LoggerServer server(2223); return server; }

		private:
			LoggerServer(const unsigned short port);
			~LoggerServer();
			void Work(Network::TcpConnection* connection) override;

			volatile unsigned char run;
			std::vector<LoggerClient*> clients;
			std::vector<Logger*> loggers;
	};
}
