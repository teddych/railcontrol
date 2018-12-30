#include "Logger/Logger.h"

using std::string;

namespace Logger
{
	Logger::Logger(LoggerServer& server, const string& component)
	:	server(server),
	 	component(component)
	{

	}

	Logger::~Logger()
	{
	}

	void Logger::Info(const string& text)
	{
		string out("Info: " + text + "\n");
		server.Send(out);
	}
}
