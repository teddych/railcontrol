#include <iostream>

#include "hardware_handler.h"
#include "manager.h"
#include "util.h"
#include "webserver.h"

Manager::Manager() {
  controllers.push_back(new WebServer(*this, 8080));
	controllers.push_back(new HardwareHandler(*this));
}

Manager::~Manager() {
  for (auto control : controllers) {
    delete control;
  }
}

void Manager::getProtocolAddress(const locoID_t locoID, hardwareControlID_t& hardwareControlID, protocol_t& protocol, address_t& address) {
	hardwareControlID = 0;
	protocol = PROTOCOL_DCC;
	address = 1228;
}

void Manager::locoSpeed(const controlID_t control_id, const locoID_t loco_id, const speed_t speed) {
  for (auto control : controllers) {
    control->locoSpeed(control_id, loco_id, speed);
  }
}

