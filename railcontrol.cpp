#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>              // dlclose
#include <iostream>
#include <unistd.h>		//close;
#include <vector>

#include "dynamic.h"
#include "hardware/hardware.h"
#include "logging.h"
#include "util.h"

using std::vector;
using std::string;


int main (int argc, char* argv[]) {
  xlog("Starting railcontrol");

	vector<int> used_hardware;
	used_hardware.push_back(HARDWARE_ID_CS2);
	used_hardware.push_back(HARDWARE_ID_VIRT);
	dynamic hardware_loader;
	void* dlhandle;
	create_hardware_t* create_hardware;
	destroy_hardware_t* destroy_hardware;
	int symbols_valid = !hardware_loader.symbols(HARDWARE_ID_CS2, &dlhandle, &create_hardware, &destroy_hardware);
	if (symbols_valid) {
		hardware::hardware* hardware = create_hardware();
		std::string name = hardware->name();
		xlog("Starting %s", name.c_str());
		xlog("Ending %s", name.c_str());
		destroy_hardware(hardware);
		dlclose(dlhandle);
	}
	else {
		xlog("Unable to load library");
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


