#include <arpa/inet.h>
#include <cstdarg>    // va_* in xlog
#include <cstdio>     // printf
#include <cstdlib>    // exit(0);
#include <cstring>    // memset
#include <iostream>   // cout
#include <sstream>
#include <string>
#include <sys/time.h> // gettimeofday
#include <unistd.h>   // close;

#include "Network/Select.h"
#include "Utils/Utils.h"

using std::cout;
using std::endl;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

namespace Utils
{
	void Utils::ReplaceString(std::string& str, const std::string& from, const std::string& to)
	{
		while (true)
		{
			size_t start_pos = str.find(from);
			if (start_pos == string::npos)
			{
				return;
			}
			str.replace(start_pos, from.length(), to);
		}
	}

	void Utils::SplitString(const std::string& str_in, const std::string &delimiter, std::vector<string> &list)
	{
		size_t length_delim = delimiter.length();
		string str(str_in);
		while (true)
		{
			size_t pos = str.find(delimiter);
			list.push_back(str.substr(0, pos));
			if (pos == string::npos)
			{
				return;
			}
			str = string(str.substr(pos + length_delim, string::npos));
		}
	}

	std::string Utils::GetStringMapEntry(const std::map<std::string, std::string>& map, const std::string& key, const std::string& defaultValue)
	{
		if (map.count(key) == 0)
		{
			return defaultValue;
		}
		return map.at(key);
	}

	int Utils::GetIntegerMapEntry(const std::map<std::string, std::string>& map, const std::string& key, const int defaultValue)
	{
		if (map.count(key) == 0)
		{
			return defaultValue;
		}
		return Utils::Utils::StringToInteger(map.at(key), defaultValue);
	}

	bool Utils::GetBoolMapEntry(const std::map<std::string, std::string>& map, const std::string& key, const bool defaultValue)
	{
		if (map.count(key) == 0)
		{
			return defaultValue;
		}
		string value = map.at(key);
		return (value.compare("true") == 0 || value.compare("on") == 0 || value.compare("1") == 0 || key.compare(value) == 0);
	}

	string Utils::ToStringWithLeadingZeros(const unsigned int number, const unsigned char chars)
	{
		string out = to_string(number);
		while (out.size() < chars)
		{
			out.insert(0, "0");
		}
		return out;
	}

	int Utils::StringToInteger(const std::string& value, const int defaultValue)
	{
		size_t valueSize = value.size();
		if (valueSize == 0)
		{
			return defaultValue;
		}

		char* end;
		const char* start = value.c_str();
		long longValue = std::strtol(start, &end, 10);
		if (errno == ERANGE || start == end)
		{
			return defaultValue;
		}
		if (longValue > INT_MAX || longValue < INT_MIN)
		{
			return defaultValue;
		}
		return static_cast<int>(longValue);
	}

	int Utils::StringToInteger(const std::string& value, const int min, const int max)
	{
		int intValue = StringToInteger(value, min);

		if (intValue < min)
		{
			return min;
		}

		if (intValue > max)
		{
			return max;
		}

		return intValue;
	}

	bool Utils::StringToBool(const std::string& value)
	{

		int intValue = StringToInteger(value);
		return intValue != 0;
	}
}
