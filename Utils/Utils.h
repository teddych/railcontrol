#pragma once

#include <climits>
#include <map>
#include <string>
#include <vector>

namespace Utils
{
	class Utils
	{
		public:
			static void ReplaceString(std::string& str, const std::string& from, const std::string& to);
			static void SplitString(const std::string& str, const std::string& delimiter, std::vector<std::string>& list);
			static std::string GetStringMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const std::string& defaultValue = "");
			static int GetIntegerMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const int defaultValue = 0);
			static bool GetBoolMapEntry(const std::map<std::string,std::string>& map, const std::string& key, const bool defaultValue = false);
			static std::string ToStringWithLeadingZeros(const unsigned int number, const unsigned char chars);
			static int StringToInteger(const std::string& value) { return StringToInteger(value, 0, INT_MAX); }
			static int StringToInteger(const std::string& value, const int defaultValue);
			static int StringToInteger(const std::string& value, const int min, const int max);
			static bool StringToBool(const std::string& value);
			static void SetThreadName(const std::string& name) { SetThreadName(name.c_str()); }
			static void SetThreadName(__attribute__((unused)) const char* name)
			{
#if __GLIBC__ > 2 || (__GLIBC__ == 2 && __GLIBC_MINOR__ >= 12)
				pthread_setname_np(pthread_self(), name);
#endif
			}
	};
}
