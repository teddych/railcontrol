#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>              // dlclose
#include <iostream>
#include <unistd.h>		//close;
#include <vector>

#include "control.h"
#include "hardware/hardware.h"
#include "hardware_properties.h"
#include "manager.h"
#include "railcontrol.h"
#include "util.h"
#include "webserver.h"

using std::vector;
using std::string;

static unsigned int run;

void stop_all() {
	run = false;
}

int main (int argc, char* argv[]) {
	run = true;
  xlog("Starting railcontrol");

	// init manager
	manager m;

	while(run) {
		sleep(1);
	}

  xlog("Ending railcontrol");

	// manager is cleaned up implicitly while leaving scope

  return 0;
}


