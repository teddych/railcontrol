#include <iostream>

#include "hardware_properties.h"
#include "manager.h"
#include "util.h"
#include "webserver/webserver.h"

using datamodel::Loco;
using storage::Storage;
using webserver::WebServer;

Manager::Manager() :
	storage(NULL) {

  controllers.push_back(new WebServer(*this, 8080));
	//controllers.push_back(new HardwareHandler(*this));

	struct Params params;
	params.name = "Virtuelle Zentrale";
	params.ip = "";
	hardwareControlID_t nextControlID = 0;
	controllers.push_back(new HardwareProperties(HARDWARE_ID_VIRT, nextControlID++, params));

	storage = new Storage();
	locos = storage->allLocos();
	for (auto loco : locos) {
		xlog("Loco %s loaded", loco->name.c_str());
	}
}

Manager::~Manager() {
  for (auto control : controllers) {
    delete control;
  }
	for (auto loco : locos) {
		delete loco;
	}
	delete storage;
	storage = NULL;
}

void Manager::go(const controlID_t controlID) {
  for (auto control : controllers) {
		control->go(controlID);
	}
}

void Manager::stop(const controlID_t controlID) {
  for (auto control : controllers) {
		control->stop(controlID);
	}
}

bool Manager::getProtocolAddress(const locoID_t locoID, hardwareControlID_t& hardwareControlID, protocol_t& protocol, address_t& address) {
	hardwareControlID = 0;
	protocol = PROTOCOL_DCC;
	address = 1228;
	return true;
}

void Manager::locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) {
  for (auto control : controllers) {
    control->locoSpeed(controlID, locoID, speed);
  }
}

