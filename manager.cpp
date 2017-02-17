#include <iostream>

#include "manager.h"
#include "railcontrol.h"
#include "hardware/hardware_handler.h"
#include "hardware/hardware_params.h"
#include "util.h"
#include "webserver/webserver.h"

using datamodel::Loco;
using hardware::HardwareHandler;
using hardware::HardwareParams;
using std::map;
using std::string;
using storage::StorageHandler;
using storage::StorageParams;
using webserver::WebServer;

Manager::Manager(Config& config) :
	storage(NULL) {

	StorageParams storageParams;
	storageParams.module = config.getValue("dbengine", "sqlite");
	storageParams.filename = config.getValue("dbfilename", "/tmp/railcontrol.db");
	storage = new StorageHandler(storageParams);

  controllers.push_back(new WebServer(*this, config.getValue("webserverport", 80)));


	xlog("Loading HardwareParams");
	storage->allHardwareParams(hardwareParams);
	xlog("Loading Controllers");
	controllers.push_back(new HardwareHandler(*this, *(hardwareParams[1])));
	/*
	for (auto hardwareParam : hardwareParams) {
		xlog("Loading Controller %s", hardwareParam.second->name);
		controllers.push_back(new HardwareHandler(*this, *(hardwareParam.second)));
		xlog("Controller %s loaded", hardwareParam.second->name);
	}
	*/
	xlog("Controllers loaded");

/*
	HardwareParams hardwareParamsVirt;
	hardwareParamsVirt.name = "Virtuelle Zentrale";
	hardwareParamsVirt.ip = "";
	controlID_t nextControlID = 0;
	controllers.push_back(new HardwareHandler(*this, HARDWARE_ID_VIRT, nextControlID++, hardwareParamsVirt));
	*/

/*
	Loco newloco1(1, "My Loco", 4, 1200);
	storage->loco(newloco1);

	Loco newloco2(2, "Your Loco", 4, 1201);
	storage->loco(newloco2);
*/

	storage->allLocos(locos);
	for (auto loco : locos) {
		xlog("Loaded loco %s", loco.second->name.c_str());
	}
}

Manager::~Manager() {
	for (auto loco : locos) {
		delete loco.second;
	}
  for (auto params : hardwareParams) {
    delete params.second;
  }
  for (auto control : controllers) {
    delete control;
  }
	delete storage;
	storage = NULL;
}

void Manager::go(const managerID_t managerID) {
  for (auto control : controllers) {
		control->go(managerID);
	}
}

void Manager::stop(const managerID_t managerID) {
  for (auto control : controllers) {
		control->stop(managerID);
	}
}

bool Manager::getProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const {
	if (locos.count(locoID) < 1) {
		controlID = 0;
		protocol = PROTOCOL_NONE;
		address = 0;
		return false;
	}
	Loco* loco = locos.at(locoID);
	controlID = loco->controlID;
	protocol = loco->protocol;
	address = loco->address;
	return true;
}

void Manager::locoSpeed(const managerID_t controlID, const locoID_t locoID, const speed_t speed) {
  for (auto control : controllers) {
    control->locoSpeed(controlID, locoID, speed);
  }
}

const std::map<locoID_t,datamodel::Loco*>& Manager::locoList() const {
	return locos;
}
