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
	storage(NULL),
	unknownLoco("Unknown Loco") {

	StorageParams storageParams;
	storageParams.module = config.getValue("dbengine", "sqlite");
	storageParams.filename = config.getValue("dbfilename", "/tmp/railcontrol.db");
	storage = new StorageHandler(storageParams);

  controllers.push_back(new WebServer(*this, config.getValue("webserverport", 80)));

	//HardwareParams newHardwareParams(1, 1, "Virtuelle Zentrale", "");
	//storage->hardwareParams(newHardwareParams);

	xlog("Loading Controllers");
	storage->allHardwareParams(hardwareParams);
	for (auto hardwareParam : hardwareParams) {
		hardwareParam.second->manager = this;
		controllers.push_back(new HardwareHandler(*this, hardwareParam.second));
	}
	xlog("Controllers loaded");

/*
	Loco newloco1(1, "Re 460 Teddy", 1, 5, 1119);
	storage->loco(newloco1);

	Loco newloco2(2, "ICN", 1, 5, 1118);
	storage->loco(newloco2);
	*/

	storage->allLocos(locos);
	for (auto loco : locos) {
		xlog("Loaded loco %i/%s", loco.second->locoID, loco.second->name.c_str());
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

void Manager::saveHardware(const controlID_t& controlID, const hardwareID_t& hardwareID, const std::string& name, const std::string& ip) {
	HardwareParams* params;
	if (hardwareParams.count(controlID) == 1) {
		params = hardwareParams.at(controlID);
		params->name = name;
		params->ip = ip;
		// FIXME: reload hardware
	}
	else {
		params = new HardwareParams(controlID, hardwareID, name, ip);
		hardwareParams[controlID] = params;
		controllers.push_back(new HardwareHandler(*this, params));
	}
	if (storage && params) {
		storage->hardwareParams(*params);
	}
}

void Manager::deleteHardware(controlID_t controlID) {
	if (hardwareParams.count(controlID) == 1) {
		//vector<>::iterator control =
		// FIXME: deleteHardware not implemented
		// FIXME: check if there are no elements on this Hardware
	}
}

HardwareParams* Manager::getHardware(controlID_t controlID) {
	if (hardwareParams.count(controlID) == 1) {
		return hardwareParams.at(controlID);
	}
	return NULL;
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

const std::string& Manager::getLocoName(const locoID_t locoID) {
	if (locos.count(locoID) == 1) {
		return locos.at(locoID)->name;
	}
	return unknownLoco;
}

void Manager::locoSpeed(const managerID_t managerID, const protocol_t protocol, const address_t address, const speed_t speed) {
	for (auto loco : locos) {
		if (loco.second->protocol == protocol && loco.second->address == address) {
			locoSpeed(managerID, loco.first, speed);
			return;
		}
	}
}

void Manager::locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) {
  for (auto control : controllers) {
    control->locoSpeed(managerID, locoID, speed);
  }
}

void Manager::locoDirection(const managerID_t managerID, const protocol_t protocol, const address_t address, const direction_t direction) {
	for (auto loco : locos) {
		if (loco.second->protocol == protocol && loco.second->address == address) {
			locoDirection(managerID, loco.first, direction);
			return;
		}
	}
}
void Manager::locoDirection(const managerID_t managerID, const locoID_t locoID, const direction_t direction) {
	for (auto control : controllers) {
		control->locoDirection(managerID, locoID, direction);
	}
}

void Manager::locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) {
	for (auto control : controllers) {
		control->locoFunction(managerID, locoID, function, on);
	}
}

const std::map<locoID_t,datamodel::Loco*>& Manager::locoList() const {
	return locos;
}

void Manager::feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) {
  for (auto control : controllers) {
		control->feedback(managerID, pin, state);
	}
}
