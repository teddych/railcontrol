#ifndef CONFIG_H
#define CONFIG_H

#include <map>
#include <string>

class Config {
	public:
		Config(std::string fileName);
		std::string getValue(const std::string& key, const std::string& value);
		int getValue(const std::string& key, const int& value);

	private:
		std::map<std::string,std::string> config;
};

#endif // CONFIG_H
