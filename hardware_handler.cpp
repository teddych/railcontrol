#include "hardware_handler.h"
#include "util.h"

using std::string;

HardwareHandler::HardwareHandler(Manager& m) :
	Control(CONTROL_ID_HARDWARE),
	nextHardwareControlID(0),
	m(m) {

	// create hardware
	hardware.push_back(new HardwareProperties(HARDWARE_ID_VIRT, nextHardwareControlID++));
	hardware.push_back(new HardwareProperties(HARDWARE_ID_CS2, nextHardwareControlID++));

	// starting hardware
	for(auto property : hardware) {
		property->start();
		std::string name = property->name();
		xlog("Starting %s", name.c_str());
	}
}

HardwareHandler::~HardwareHandler() {
	// Stopping hardware
	for(auto property : hardware) {
		std::string name = property->name();
		xlog("Stopping %s", name.c_str());
		property->stop();
		delete property;
	}
	// vector hardware is cleaned up implicitly while leaving scope (class hardware_handler)
}

/*
loco_id_t hardware_handler::get_loco_id(protocol_t protocol, address_t address) {
  return 1;
}

hardware_control_id_t hardware_handler::get_hardware_control_id(loco_id_t loco_id) {
  return 1;
}
*/

hardwareControlID_t HardwareHandler::getHardwareControlID(protocol_t protocol, address_t address) {
  return 0;
}

void HardwareHandler::locoSpeed(const controlID_t controlID, const protocol_t protocol, const address_t address, const speed_t speed) {
  if (controlID != CONTROL_ID_HARDWARE) {
    hardwareControlID_t hardwareControlID = getHardwareControlID(protocol, address);
		hardware[hardwareControlID]->locoSpeed(protocol, address, speed);
  }
}
