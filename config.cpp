#include <fstream>
#include <istream> // std::ws
#include <sstream>

#include "config.h"
#include "Logger/Logger.h"
#include "Utils/Utils.h"

using std::map;
using std::string;

Config::Config(std::string fileName)
{
	Logger::Logger* logger = Logger::LoggerServer::Instance().GetLogger("Main");
	// read config values
	logger->Info("Reading config file {0}", fileName);

	std::ifstream configFile;
	configFile.open(fileName);
	if (!configFile.is_open())
	{
		logger->Warning("Unable to open configfile");
		return;
	}

	for (string line; std::getline(configFile, line); )
	{
		std::istringstream iss(line);
		string configKey;
		string eq;
		string configValue;

		if (configKey[0] == '#')
		{
			continue;
		}

		bool error = (!(iss >> configKey >> eq >> configValue >> std::ws) || eq != "=" || iss.get() != EOF);
		if (error == true)
		{
			continue;
		}

		config[configKey] = configValue;
	}
	configFile.close();

	for(auto option : config)
	{
		logger->Info("Parameter found in config file: {0} = {1}", option.first, option.second);
	}
}

const string& Config::getValue(const string& key, const string& defaultValue)
{
	if (config.count(key) != 1)
	{
		return defaultValue;
	}
	return config[key];
}

int Config::getValue(const string& key, const int& defaultValue)
{
	if (config.count(key) != 1)
	{
		return defaultValue;
	}
	return Utils::Utils::StringToInteger(config[key], defaultValue);
}

