#include <iomanip>
#include <sstream>
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

	void Logger::Replace(string& workString,
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

	void Logger::Hex(const unsigned char* input, const size_t size)
	{
		std::stringstream output;
		for (size_t index = 0; index < size; ++index)
		{
			if ((index & 0x0F) == 0)
			{
				output << "0x" << std::setfill('0') << std::setw(4) << std::hex << index << "   ";
			}

			output << std::setfill('0') << std::setw(2) << std::hex << static_cast<unsigned int>(input[index]);

			size_t next = index + 1;
			if ((next & 0x0F) == 0)
			{
				Debug(output.str());
				output = std::stringstream(); // clear output
				if (next == size)
				{
					return;
				}
				continue;
			}
			output << " ";
			if ((next & 0x07) == 0)
			{
				output << "  ";
			}
		}
		Debug(output.str());
	}
}

