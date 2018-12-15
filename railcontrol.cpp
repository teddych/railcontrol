#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>              // dlclose
#include <iostream>
#include <signal.h>
#include <sstream>
#include <unistd.h>		//close;
#include <vector>

#include "hardware/HardwareHandler.h"
#include "manager.h"
#include "network/Select.h"
#include "railcontrol.h"
#include "util.h"
#include "webserver/webserver.h"

using std::vector;
using std::string;

static volatile unsigned int runRailcontrol;

void stopRailControlSignal(int signo) {
	xlog("Stopping railcontrol requested by signal %i", signo);
	runRailcontrol = false;
}

void stopRailControlWebserver() {
	xlog("Stopping railcontrol requested by a webclient");
	runRailcontrol = false;
}

void stopRailControlConsole() {
	xlog("Stopping railcontrol requested by a console client");
	runRailcontrol = false;
}

int main (int argc, char* argv[]) {
	signal(SIGINT, stopRailControlSignal);
	signal(SIGTERM, stopRailControlSignal);

	runRailcontrol = true;
	xlog("Starting railcontrol");

	string configfile;
	if (argc == 2) {
		configfile.append(argv[1]);
	}
	else {
		configfile.append("railcontrol.conf");
	}

	Config config(configfile);

	// init manager that does all the stuff in a seperate thread
	Manager m(config);

	// wait for q followed by \n or SIGINT or SIGTERM
	char input = 0;

	struct timeval tv;
	fd_set set;

	do {
		tv.tv_sec = 1;
		tv.tv_usec = 0;
		// Zero out the fd_set - make sure it's pristine
		FD_ZERO(&set);

		// Set the FD that we want to read
		FD_SET(STDIN_FILENO, &set); //STDIN_FILENO is 0

		// select takes the last file descriptor value + 1 in the fdset to check,
		// the fdset for reads, writes, and errors.  We are only passing in reads.
		// the last parameter is the timeout.  select will return if an FD is ready or 
		// the timeout has occurred
		int ret = TEMP_FAILURE_RETRY(select(FD_SETSIZE, &set, NULL, NULL, &tv));

		// only read STDIN if there really is something to read
		if (ret > 0 && FD_ISSET(STDIN_FILENO, &set)) {
			read(STDIN_FILENO, &input, sizeof(input));
		}
	} while (input != 'q' && runRailcontrol);

	xlog("Stopping railcontrol");

	// manager is cleaned up implicitly while leaving scope

	return 0;
}

