#pragma once

#include <string>

#include "Logger/LoggerServer.h"
#include "network/TcpServer.h"

namespace Logger
{
	class Logger
	{
		public:
			Logger(LoggerServer& server, const std::string& component)
			:	server(server),
			 	component(component)
			{}

			~Logger() {};

			bool IsComponent(const std::string& component) { return component.compare(this->component) == 0; }
			void Log(const std::string& type, const std::string& text);
			void Error(const std::string& text)
			{
				Log("Error", text);
			}

			void Warning(const std::string& text)
			{
				Log("Warning", text);
			}

			void Info(const std::string& text)
			{
				Log("Info", text);
			}

			void Debug(const std::string& text)
			{
				Log("Debug", text);
			}

			LoggerServer& server;
			const std::string component;
	};
}
