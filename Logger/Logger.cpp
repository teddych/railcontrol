#include <sys/time.h> // gettimeofday

#include "Logger/Logger.h"

using std::string;

namespace Logger
{
	string Logger::DateTime()
	{
		char buffer[27];
		struct timeval timestamp;
		gettimeofday(&timestamp, NULL);
		struct tm tm;
		gmtime_r(&timestamp.tv_sec, &tm);
		strftime(buffer, sizeof(buffer), "%F %T.", &tm);
		snprintf(buffer + 20, sizeof(buffer) - 20, "%06li", timestamp.tv_usec);
		return string(buffer);
	}
}

