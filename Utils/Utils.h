#pragma once

#include <climits>
#include <map>
#include <string>
#include <vector>

// replace string from with to in str
void str_replace(std::string& str, const std::string& from, const std::string& to);

// split string in vector<string>
void str_split(const std::string& str, const std::string& delimiter, std::vector<std::string>& list);

std::string GetStringMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const std::string& defaultValue = "");
int GetIntegerMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const int defaultValue = 0);
bool GetBoolMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const bool defaultValue = false);

std::string toStringWithLeadingZeros(const unsigned int number, const unsigned char chars);

namespace Utils
{
	class Utils
	{
		public:
			static int StringToInteger(const std::string& value) { return StringToInteger(value, 0, INT_MAX); }
			static int StringToInteger(const std::string& value, const int defaultValue);
			static int StringToInteger(const std::string& value, const int min, const int max);
			static bool StringToBool(const std::string& value);
	};
}
