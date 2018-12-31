#include <sys/time.h> // gettimeofday

#include "Logger/Logger.h"

using std::string;

namespace Logger
{
	void Logger::Log(const string& type, const string& text)
	{
		char buffer[27];

		// Get the current time
		struct timeval timestamp;
		gettimeofday(&timestamp, NULL);

		// Convert it to local time representation
		struct tm tm;
		gmtime_r(&timestamp.tv_sec, &tm);
		strftime(buffer, sizeof(buffer), "%F %T.", &tm);
		snprintf(buffer + 20, sizeof(buffer) - 20, "%06li", timestamp.tv_usec);

		string out(string(buffer) + ": " + type + ": " + component + ": " + text + "\n");
		server.Send(out);
	}
}

