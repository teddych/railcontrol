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

			template<typename... Args> void Error(Args... args)
			{
				Log("Error", args...);
			}

			template<typename... Args> void Warning(Args... args)
			{
				Log("Warning", args...);
			}

			template<typename... Args> void Info(Args... args)
			{
				Log(std::string("Info"), args...);
			}

			template<typename... Args> void Debug(Args... args)
			{
				Log("Debug", args...);
			}

			LoggerServer& server;
			const std::string component;

		private:
			std::string DateTime();
			template<typename T> std::string LogVariadic(T v) { return v; }
			template<typename T, typename... Args> std::string LogVariadic(T v, Args... args) { return v + LogVariadic(args...); }

			template<typename... Args> void Log(const std::string& type, Args... args)
			{
				server.Send(DateTime() + ": " + type + ": " + component + ": " + LogVariadic(args...) + "\n");
			}
	};
}
