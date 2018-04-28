#include <fstream>
#include <istream> // std::ws
#include <sstream>

#include "config.h"
#include "util.h"

using std::map;
using std::string;

Config::Config(std::string fileName) {
	// read config values
	xlog("Reading config file %s", fileName.c_str());

	std::ifstream configFile;
	configFile.open(fileName);
	if (!configFile.is_open()) {
		xlog("Unable to open configfile");
		return;
	}

	for (string line; std::getline(configFile, line); ) {
		std::istringstream iss(line);
		string configKey;
		string eq;
		string configValue;
		bool error = false;

		if (!(iss >> configKey >> eq >> configValue >> std::ws) || eq != "=" || iss.get() != EOF) {
			error = true;
		}
		if (configKey[0] == '#') {
			continue;
		}

		if (!error) {
			config[configKey] = configValue;
		}
	}
	configFile.close();

	for(auto option : config) {
		xlog("Parameter found in config file: %s = %s", option.first.c_str(), option.second.c_str());
	}
}

const string& Config::getValue(const string& key, const string& value) {
	if (config.count(key) == 1) {
		return config[key];
	}
	return value;
}

int Config::getValue(const string& key, const int& value) {
	if (config.count(key) == 1) {
		return std::stoi(config[key]);
	}
	return value;
}

