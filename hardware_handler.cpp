#include "hardware_handler.h"
#include "util.h"

using std::string;

hardware_handler::hardware_handler(manager& m) :
	control(CONTROL_ID_HARDWARE),
	next_hardware_control_id(0),
	m(m) {

	// create hardware
	hardware.push_back(new hardware_properties(HARDWARE_ID_VIRT, next_hardware_control_id++));
	hardware.push_back(new hardware_properties(HARDWARE_ID_CS2, next_hardware_control_id++));

	// starting hardware
	for(auto property : hardware) {
		property->start();
		std::string name = property->name();
		xlog("Starting %s", name.c_str());
	}
}

hardware_handler::~hardware_handler() {
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

hardware_control_id_t hardware_handler::get_hardware_control_id(protocol_t protocol, address_t address) {
  return 0;
}

void hardware_handler::loco_speed(const control_id_t control_id, const protocol_t protocol, const address_t address, const speed_t speed) {
  if (control_id != CONTROL_ID_HARDWARE) {
    hardware_control_id_t hardware_control_id = get_hardware_control_id(protocol, address);
		hardware[hardware_control_id]->loco_speed(protocol, address, speed);
  }
}
