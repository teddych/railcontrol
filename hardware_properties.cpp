#include <dlfcn.h>              // dl*
#include <sstream>

#include "hardware_properties.h"
#include "util.h"

using std::string;


hardware_properties::hardware_properties(hardware_id_t hardware_id, control_id_t control_id) :
	hardware_id(hardware_id),
	control_id(control_id),
	create_hardware(NULL),
	destroy_hardware(NULL),
	instance(NULL),
	dlhandle(NULL) {

  // generate symbol and library names
  char* error;
	string symbol = hardware_symbols[hardware_id];
	std::stringstream ss;
	ss << "hardware/" << symbol << ".so";
  xlog("ID: %i Symbol: %s", hardware_id, symbol.c_str());

  dlhandle = dlopen(ss.str().c_str(), RTLD_LAZY);
  if (!dlhandle) {
    xlog("Can not open library: %s", dlerror());
		return;
  }

	// look for symbol create_*
  ss.str(std::string());
	ss << "create_" << symbol;
	const char* s = ss.str().c_str();
  create_hardware_t* new_create_hardware = (create_hardware_t*)dlsym(dlhandle, s);
  error = dlerror();
  if (error) {
    xlog("Unable to find symbol %s", s);
		return;
  }

	// look for symbol destroy_*
  ss.str(std::string());
	ss << "destroy_" << symbol;
	s = ss.str().c_str();
  destroy_hardware_t* new_destroy_hardware = (destroy_hardware_t*)dlsym(dlhandle, ss.str().c_str());
  error = dlerror();
  if (error) {
    xlog("Unable to find symbol %s", s);
		return;
  }

	// return valid symbols
	create_hardware = new_create_hardware;
	destroy_hardware = new_destroy_hardware;
	return;
}

hardware_properties::~hardware_properties() {
	if (dlhandle) {
		dlclose(dlhandle);
		dlhandle = NULL;
	}
}

std::string hardware_properties::name() const {
	if (instance) {
		return instance->name();
	}
	return "Unknown, not running";
}

int hardware_properties::start() {
	instance = create_hardware();
	xlog("X");
	return 0;
}

int hardware_properties::stop() {
	xlog("Y");
	if (instance) {
		destroy_hardware(instance);
		instance = NULL;
	}
	return 0;
}
