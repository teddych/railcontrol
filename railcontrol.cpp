#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>              // dlclose
#include <iostream>
#include <signal.h>
#include <sstream>
#include <unistd.h>		//close;
#include <vector>

#include "hardware/hardware_handler.h"
#include "manager.h"
#include "railcontrol.h"
#include "util.h"
#include "webserver/webserver.h"

using std::vector;
using std::string;

static unsigned int run;

void stopRailControl(__attribute__((unused))int signo) {
	run = false;
}

int main (int argc, char* argv[]) {
	signal(SIGINT, stopRailControl);
	signal(SIGTERM, stopRailControl);
	run = true;
  xlog("Starting railcontrol");

	Config config("railcontrol.conf");;

	// init manager
	Manager m(config);

	while(run) {
		sleep(1);
	}

  xlog("Stopping railcontrol");

	// manager is cleaned up implicitly while leaving scope

  return 0;
}


