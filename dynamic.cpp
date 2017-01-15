#include <dlfcn.h>              // dl*
#include <sstream>

#include "util.h"
#include "dynamic.h"

using std::string;


int dynamic::symbols(hardware_id_t hardware_id, void** dlhandle, create_hardware_t** create_hardware, destroy_hardware_t** destroy_hardware) {
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

