#include "hardware_handler.h"
#include "util.h"

using std::string;

HardwareHandler::HardwareHandler(Manager& manager) :
	Control(CONTROL_ID_HARDWARE),
	nextHardwareControlID(0),
	manager(manager) {

	// create hardware
	struct Params params;
	params.name = "Virtuelle Zentrale";
	params.ip = "";
	hardware.push_back(new HardwareProperties(HARDWARE_ID_VIRT, nextHardwareControlID++, params));
//	params.name = "Reelle Zentrale";
//	hardware.push_back(new HardwareProperties(HARDWARE_ID_CS2, nextHardwareControlID++, params));

	// starting hardware
	for(auto property : hardware) {
		std::string name = property->getName();
		xlog("Starting %s", name.c_str());
	}
}

HardwareHandler::~HardwareHandler() {
	// Stopping hardware
	for(auto property : hardware) {
		std::string name = property->getName();
		xlog("Stopping %s", name.c_str());
		delete property;
	}
	// vector hardware is cleaned up implicitly while leaving scope (class hardware_handler)
}

void HardwareHandler::go(const controlID_t controlID) {
  if (controlID != CONTROL_ID_HARDWARE) {
	/*
		for(auto property : hardware) {
			property->go();
		}
	*/
	}
}

void HardwareHandler::stop(const controlID_t controlID) {
  if (controlID != CONTROL_ID_HARDWARE) {
	/*
		for(auto property : hardware) {
			property->go();
		}
	*/
	}
}

void HardwareHandler::locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) {
  if (controlID != CONTROL_ID_HARDWARE) {
    hardwareControlID_t hardwareControlID;
    protocol_t protocol;
    address_t address;
    manager.getProtocolAddress(locoID, hardwareControlID, protocol, address);
    hardware[hardwareControlID]->locoSpeed(protocol, address, speed);
  }
}
