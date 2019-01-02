#include <iostream>

#include "Logger/Logger.h"
#include "Logger/LoggerServer.h"
#include "Logger/LoggerClient.h"
#include "util.h"

using std::string;

namespace Logger
{
	LoggerServer::~LoggerServer()
	{
		if (run == false)
		{
			return;
		}

		run = false;

		// delete all client memory
		while (clients.size())
		{
			LoggerClient* client = clients.back();
			clients.pop_back();
			delete client;
		}
	}

	void LoggerServer::Work(Network::TcpConnection* connection)
	{
		clients.push_back(new LoggerClient(connection));
	}

	Logger* LoggerServer::GetLogger(const std::string& component)
	{
		for (auto logger : loggers)
		{
			if (logger->IsComponent(component))
			{
				return logger;
			}
		}
		Logger* logger = new Logger(*this, component);
		loggers.push_back(logger);
		return logger;
	}

	void LoggerServer::Send(const std::string& text)
	{
		std::cout << text << std::flush;
		for(auto client : clients)
		{
			client->Send(text);
		}
	}
}
