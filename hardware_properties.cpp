#include "hardware_properties.h"

#include <dlfcn.h>              // dl*
#include <sstream>

#include "control.h"
#include "util.h"

using std::string;


HardwareProperties::HardwareProperties(const hardware_id_t hardware_id, const hardwareControlID_t hardware_control_id, const struct Params& params) :
  Control(CONTROL_ID_HARDWARE),
	hardwareID(hardware_id),
	hardwareControlID(hardware_control_id),
	createHardware(NULL),
	destroyHardware(NULL),
	instance(NULL),
	dlhandle(NULL),
	params(params) {

  // generate symbol and library names
  char* error;
	string symbol = hardwareSymbols[hardware_id];
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
	createHardware = new_create_hardware;
	destroyHardware = new_destroy_hardware;
	return;
}

HardwareProperties::~HardwareProperties() {
	if (instance) {
		destroyHardware(instance);
		instance = NULL;
	}
	if (dlhandle) {
		dlclose(dlhandle);
		dlhandle = NULL;
	}
}

std::string HardwareProperties::getName() const {
	if (instance) {
		return instance->getName();
	}
	return "Unknown, not running";
}

void HardwareProperties::start() {
	if (createHardware) {
		instance = createHardware(params);
	}
}

void HardwareProperties::stop() {
	if (instance) {
		destroyHardware(instance);
		instance = NULL;
	}
}

