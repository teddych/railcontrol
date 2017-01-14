#include <arpa/inet.h>
#include <cstdarg>              // va_* in xlog
#include <cstdio>		//printf
#include <cstdlib>		//exit(0);
#include <cstring>		//memset
#include <dlfcn.h>              // dl*
#include <iostream>
#include <sstream>
#include <unistd.h>		//close;
#include <vector>

#include "hardware.h"
#include "hardware/hardware.h"
#include "logging.h"
#include "util.h"

using std::vector;
using std::string;


/*
string get_hardware_string(hardware_id_t hardware_id) {
	
}
*/

int get_hardware_symbols(hardware_id_t hardware_id, void** dlhandle, create_hardware_t** create_hardware, destroy_hardware_t** destroy_hardware) {
/*
std::map<int, std::string> hardware_symbols;
hardware_symbols[HARDWARE_ID_VIRT] = "virt";
hardware_symbols[HARDWARE_ID_CS2] = "cs2";
*/
  char* error;
	string symbol = hardware_symbols[hardware_id];
	std::stringstream ss;
	ss << "hardware/" << symbol << ".so";
  xlog("ID: %i Symbol: %s", hardware_id, symbol.c_str());

	xlog(ss.str().c_str());
  *dlhandle = dlopen(ss.str().c_str(), RTLD_LAZY);
  if (!*dlhandle) {
    printf("Can not open library: %s\n", dlerror());
		return 1;
  }

  ss.str(std::string());
	ss << "create_" << symbol;
	xlog(ss.str().c_str());
  create_hardware_t* new_create_hardware = (create_hardware_t*)dlsym(*dlhandle, ss.str().c_str());
  error = dlerror();
  if (error) {
    printf("Unable to find symbol create_cs2\n");
		return 1;
  }

  ss.str(std::string());
	ss << "destroy_" << symbol;
	xlog(ss.str().c_str());
  destroy_hardware_t* new_destroy_hardware = (destroy_hardware_t*)dlsym(*dlhandle, ss.str().c_str());
  error = dlerror();
  if (error) {
    printf("Unable to find symbol destroy_cs2\n");
		return 1;
  }

	*create_hardware = new_create_hardware;
	*destroy_hardware = new_destroy_hardware;
	return 0;
}

int main (int argc, char* argv[]) {
  xlog("Starting");

	vector<int> used_hardware;
	used_hardware.push_back(HARDWARE_ID_CS2);
	used_hardware.push_back(HARDWARE_ID_VIRT);
	void* dlhandle;
	create_hardware_t* create_hardware;
	destroy_hardware_t* destroy_hardware;
	int symbols_valid = !get_hardware_symbols(HARDWARE_ID_CS2, &dlhandle, &create_hardware, &destroy_hardware);
	if (symbols_valid) {
		hardware::hardware* hardware = create_hardware();
		std::string name = hardware->name();
		xlog(name.c_str());
		destroy_hardware(hardware);
		dlclose(dlhandle);
	}
	else {
		xlog("Unable to load library");
	}


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


