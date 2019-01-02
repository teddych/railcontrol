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

	void Logger::Replace(std::string& workString,
		const unsigned char argument,
		const std::string& value)
	{
		std::string needle = "{" + std::to_string(argument) + "}";
		size_t pos = workString.find(needle);
		if (pos == std::string::npos)
		{
			return;
		}
		workString.replace(pos, needle.size(), value);
	}
}

