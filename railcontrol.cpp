#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>              // dlclose
#include <fstream>
#include <iostream>
#include <signal.h>
#include <sstream>
#include <unistd.h>		//close;
#include <vector>

#include "config.h"
#include "hardware/hardware_handler.h"
#include "manager.h"
#include "railcontrol.h"
#include "util.h"
#include "webserver/webserver.h"

using std::vector;
using std::string;

static unsigned int run;

std::map<string,string> config;

void stopRailControl(__attribute__((unused))int signo) {
	run = false;
}

int main (int argc, char* argv[]) {
	signal(SIGINT, stopRailControl);
	signal(SIGTERM, stopRailControl);
	run = true;
  xlog("Starting railcontrol");

	// read config values
	std::ifstream configFile;
	configFile.open("railcontrol.conf");
	if (configFile.is_open()) {

		for (string line; std::getline(configFile, line); ) {
			std::istringstream iss(line);
			string configKey;
			string eq;
			string configValue;
			bool error = false;

			if (id[0] == '#') {
				continue;
			}
			else if (!(iss >> configKey >> eq >> configValue >> std::ws) || eq != "=" || iss.get() != EOF) {
				error = true;
			}

			if (!error) {
				config[configKey] = configValue;
			}
		}
		configFile.close();
	}

	for(auto option : config) {
		xlog("%s=%s", option.first.c_str(), option.second.c_str());
	}

	// init manager
	Manager m;

	while(run) {
		sleep(1);
	}

  xlog("Stopping railcontrol");

	// manager is cleaned up implicitly while leaving scope

  return 0;
}


