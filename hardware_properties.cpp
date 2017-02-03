#include <dlfcn.h>              // dl*
#include <sstream>

#include "control.h"
#include "datatypes.h"
#include "hardware_properties.h"
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

	// register  valid symbols
	createHardware = new_create_hardware;
	destroyHardware = new_destroy_hardware;

	// start control
	if (createHardware) {
		instance = createHardware(params);
	}

	return;
}

HardwareProperties::~HardwareProperties() {
	// stop control
	if (instance) {
		destroyHardware(instance);
		instance = NULL;
	}
	// close library
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

void HardwareProperties::go(const controlID_t controlID) {
  if (controlID != CONTROL_ID_HARDWARE) {
		instance->go();
	}
}

void HardwareProperties::stop(const controlID_t controlID) {
  if (controlID != CONTROL_ID_HARDWARE) {
		instance->stop();
	}
}

void HardwareProperties::locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) {
  if (controlID != CONTROL_ID_HARDWARE) {
    //hardwareControlID_t hardwareControlID = 0;
    protocol_t protocol = PROTOCOL_DCC;
    address_t address = 1028;
	/*
    manager.getProtocolAddress(locoID, hardwareControlID, protocol, address);
		*/
		instance->locoSpeed(protocol, address, speed);
  }
}
