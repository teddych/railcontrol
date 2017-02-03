#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>              // dlclose
#include <iostream>
#include <signal.h>
#include <unistd.h>		//close;
#include <vector>

#include "railcontrol.h"

#include "control.h"
#include "hardware/control_interface.h"
#include "hardware_properties.h"
#include "manager.h"
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

	// init manager
	Manager m;

	while(run) {
		sleep(1);
	}

  xlog("Stopping railcontrol");

	// manager is cleaned up implicitly while leaving scope

  return 0;
}


