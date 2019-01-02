#pragma once

#include <map>
#include <string>

class Config
{
	public:
		Config(std::string fileName);
		const std::string& getValue(const std::string& key, const std::string& defaultValue);
		int getValue(const std::string& key, const int& defaultValue);

	private:
		std::map<std::string, std::string> config;
};

