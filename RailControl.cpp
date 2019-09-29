#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>		// dlclose
#include <iostream>
#include <signal.h>
#include <sstream>
#include <unistd.h>		//close;
#include <vector>

#include "hardware/HardwareHandler.h"
#include "Logger/Logger.h"
#include "Manager.h"
#include "network/Select.h"
#include "RailControl.h"
#include "Utils/Utils.h"

using std::vector;
using std::string;

static volatile unsigned int runRailcontrol;

static volatile unsigned char stopSignalCounter;
static const unsigned char maxStopSignalCounter = 3;

void stopRailControlSignal(int signo)
{
	Logger::Logger* logger = Logger::Logger::GetLogger("Main");
	logger->Info("Stopping railcontrol requested by signal {0}", signo);
	runRailcontrol = false;
	if (++stopSignalCounter < maxStopSignalCounter)
	{
		return;
	}
	logger->Info("Received a signal kill {0} times. Exiting without saving.", maxStopSignalCounter);
	exit(1);
}

void stopRailControlWebserver()
{
	Logger::Logger::GetLogger("Main")->Info("Stopping railcontrol requested by webclient");
	runRailcontrol = false;
}

int main (int argc, char* argv[])
{
	stopSignalCounter = 0;
	signal(SIGINT, stopRailControlSignal);
	signal(SIGTERM, stopRailControlSignal);

	runRailcontrol = true;
	Logger::Logger* logger = Logger::Logger::GetLogger("Main");
	logger->Info(string("Starting railcontrol"));

	Config config(argc == 2 ? argv[1] : "railcontrol.conf");

	// init manager that does all the stuff in a seperate thread
	Manager m(config);

	// wait for q followed by \n or SIGINT or SIGTERM
	char input = 0;

	struct timeval tv;
	fd_set set;

	do
	{
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
		if (ret > 0 && FD_ISSET(STDIN_FILENO, &set))
		{
			__attribute__((unused)) size_t unused = read(STDIN_FILENO, &input, sizeof(input));
		}
	} while (input != 'q' && runRailcontrol);

	logger->Info("Stopping railcontrol");

	// manager is cleaned up implicitly while leaving scope

	return 0;
}

