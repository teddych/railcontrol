#include <iostream>

#include "manager.h"

#include "hardware/hardware_handler.h"
#include "hardware/hardware_params.h"
#include "util.h"
#include "webserver/webserver.h"

using datamodel::Loco;
using hardware::HardwareHandler;
using hardware::HardwareParams;
using storage::StorageHandler;
using storage::StorageParams;
using webserver::WebServer;

Manager::Manager() :
	storage(NULL) {

  controllers.push_back(new WebServer(*this, 8080));

	HardwareParams hardwareParamsVirt;
	hardwareParamsVirt.name = "Virtuelle Zentrale";
	hardwareParamsVirt.ip = "";
	hardwareControlID_t nextControlID = 0;
	controllers.push_back(new HardwareHandler(HARDWARE_ID_VIRT, nextControlID++, hardwareParamsVirt));

	StorageParams storageParams;
	storageParams.module = "sqlite";
	storageParams.filename = "/tmp/railcontrol.db";
	storage = new StorageHandler(storageParams);

	Loco newloco(1, "My Loco", 4, 1200);
	storage->loco(newloco);

	storage->allLocos(locos);
	for (auto loco : locos) {
		xlog("Loaded loco %s", loco.second->name.c_str());
	}
}

Manager::~Manager() {
  for (auto control : controllers) {
    delete control;
  }
	for (auto loco : locos) {
		delete loco.second;
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
	if (locos.count(locoID) < 1) {
		hardwareControlID = 0;
		protocol = PROTOCOL_NONE;
		address = 0;
		return false;
	}
	Loco* loco = locos[locoID];
	hardwareControlID = loco->hardwareControlID;
	protocol = loco->protocol;
	address = loco->address;
	return true;
}

void Manager::locoSpeed(const controlID_t controlID, const locoID_t locoID, const speed_t speed) {
  for (auto control : controllers) {
    control->locoSpeed(controlID, locoID, speed);
  }
}

