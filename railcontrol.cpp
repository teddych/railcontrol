#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>              // dlclose
#include <iostream>
#include <unistd.h>		//close;
#include <vector>

#include "hardware/hardware.h"
#include "hardware_properties.h"
#include "util.h"

using std::vector;
using std::string;

int main (int argc, char* argv[]) {
  xlog("Starting railcontrol");

	// initializing hardware
	vector<hardware_properties*> hardware;
	hardware.push_back(new hardware_properties(HARDWARE_ID_VIRT, 1));
	hardware.push_back(new hardware_properties(HARDWARE_ID_CS2, 2));

	// Starting hardware
	for(auto property : hardware) {
		property->start();
		std::string name = property->name();
		xlog("Starting %s", name.c_str());
	}

	// Stopping hardware
	for(auto property : hardware) {
		std::string name = property->name();
		xlog("Stopping %s", name.c_str());
		property->stop();
		delete property;
	}

  xlog("Ending railcontrol");

  return 0;
/*

  std::thread thread_cs2_receiver(cs2_receiver);
  std::thread thread_cs2_sender(cs2_sender);
  thread_cs2_sender.join();
  run = false;
  thread_cs2_receiver.join();
//  sleep(2);
  xlog("Ending");
*/
}


