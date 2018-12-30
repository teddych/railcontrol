#pragma once

#include <string>

#include "Logger/LoggerServer.h"
#include "network/TcpServer.h"

namespace Logger
{
	class Logger
	{
		public:
			Logger(LoggerServer& server, const std::string& component);
			~Logger();

			bool IsComponent(const std::string& component) { return component.compare(this->component) == 0; }
			//void Error(std::string& text);
			//void Warning(std::string& text);
			void Info(const std::string& text);
			//void Debug(std::string& text);

			LoggerServer& server;
			const std::string component;
	};
}
