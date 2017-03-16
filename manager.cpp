#include <iostream>

#include "manager.h"
#include "railcontrol.h"
#include "hardware/hardware_handler.h"
#include "hardware/hardware_params.h"
#include "util.h"
#include "webserver/webserver.h"

using datamodel::Accessory;
using datamodel::Block;
using datamodel::Feedback;
using datamodel::Loco;
using datamodel::Switch;
using hardware::HardwareHandler;
using hardware::HardwareParams;
using std::map;
using std::string;
using storage::StorageHandler;
using storage::StorageParams;
using webserver::WebServer;

Manager::Manager(Config& config) :
	storage(NULL),
	unknownLoco("Unknown Loco"),
	unknownAccessory("Unknown Accessory"),
	unknownFeedback("Unknown Feedback"),
	unknownBlock("Unknown Block"),
	unknownSwitch("Unknown Switch") {

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
	Loco newloco1(1, "Re 460 Teddy", 1, PROTOCOL_DCC, 1119);
	storage->loco(newloco1);

	Loco newloco2(2, "ICN", 1, PROTOCOL_DCC, 1118);
	storage->loco(newloco2);
	*/

	storage->allLocos(locos);
	for (auto loco : locos) {
		xlog("Loaded loco %i/%s", loco.second->locoID, loco.second->name.c_str());
	}

	Accessory newAccessory1(1, "Schalter 1", 1, PROTOCOL_DCC, 1, 1, ACCESSORY_STATE_ON, 200, 3, 5, 0);
	storage->accessory(newAccessory1);

	Accessory newAccessory2(2, "Schalter 2", 1, PROTOCOL_DCC, 2, 1, ACCESSORY_STATE_OFF, 200, 3, 6, 0);
	storage->accessory(newAccessory2);

	storage->allAccessories(accessories);
	for (auto accessory : accessories) {
		xlog("Loaded accessory %i/%s", accessory.second->accessoryID, accessory.second->name.c_str());
	}

	Feedback newFeedback1(1, "Rückmelder Einfahrt links", 1, 1, 4, 5, 0);
	storage->feedback(newFeedback1);

	Feedback newFeedback2(2, "Rückmelder Einfahrt rechts", 1, 2, 4, 6, 0);
	storage->feedback(newFeedback2);

	storage->allFeedbacks(feedbacks);
	for (auto feedback : feedbacks) {
		xlog("Loaded Feedback %i/%s", feedback.second->feedbackID, feedback.second->name.c_str());
	}

	Block newBlock1(1, "Block 1", 4, ROTATION_0, 5, 5, 0);
	storage->block(newBlock1);

	Block newBlock2(2, "Block 2", 4, ROTATION_90, 5, 6, 0);
	storage->block(newBlock2);

	storage->allBlocks(blocks);
	for (auto block : blocks) {
		xlog("Loaded block %i/%s", block.second->blockID, block.second->name.c_str());
	}

	Switch newSwitch1(1, "Weiche 1", 1, PROTOCOL_DCC, 3, SWITCH_LEFT, SWITCH_TURNOUT, ROTATION_90, 2, 5, 0);
	storage->saveSwitch(newSwitch1);

	Switch newSwitch2(2, "Weiche 2", 1, PROTOCOL_DCC, 4, SWITCH_RIGHT, SWITCH_STRAIGHT, ROTATION_0, 2, 6, 0);
	storage->saveSwitch(newSwitch2);

	storage->allSwitches(switches);
	// FIXME: someting with switches does not work
	for (auto mySwitch : switches) {
		xlog("Loaded Switch %i/%s", mySwitch.second->switchID, mySwitch.second->name.c_str());
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

void Manager::booster(const managerID_t managerID, const boosterStatus_t status) {
  for (auto control : controllers) {
		control->booster(managerID, status);
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

bool Manager::getAccessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const {
	if (accessories.count(accessoryID) < 1) {
		controlID = 0;
		protocol = PROTOCOL_NONE;
		address = 0;
		return false;
	}
	Accessory* accessory = accessories.at(accessoryID);
	controlID = accessory->controlID;
	protocol = accessory->protocol;
	address = accessory->address;
	return true;
}

bool Manager::getSwitchProtocolAddress(const switchID_t switchID, controlID_t& controlID, protocol_t& protocol, address_t& address) const {
	if (switches.count(switchID) < 1) {
		controlID = 0;
		protocol = PROTOCOL_NONE;
		address = 0;
		return false;
	}
	Switch* mySwitch = switches.at(switchID);
	controlID = mySwitch->controlID;
	protocol = mySwitch->protocol;
	address = mySwitch->address;
	return true;
}

const std::string& Manager::getLocoName(const locoID_t locoID) {
	if (locos.count(locoID) == 1) {
		return locos.at(locoID)->name;
	}
	return unknownLoco;
}

const std::string& Manager::getAccessoryName(const accessoryID_t accessoryID) {
	if (accessories.count(accessoryID) == 1) {
		return accessories.at(accessoryID)->name;
	}
	return unknownAccessory;
}

const std::string& Manager::getBlockName(const blockID_t blockID) {
	if (blocks.count(blockID) == 1) {
		return blocks.at(blockID)->name;
	}
	return unknownBlock;
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

void Manager::accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) {
  for (auto control : controllers) {
		control->accessory(managerID, accessoryID, state);
	}
}

void Manager::block(const managerID_t managerID, const blockID_t blockID, const blockState_t state) {
  for (auto control : controllers) {
		control->block(managerID, blockID, state);
	}
}

Block* Manager::getBlock(const blockID_t blockID) {
	std::lock_guard<std::mutex> Guard(blockMutex);
	if (blocks.count(blockID) == 1) {
		return blocks.at(blockID);
	}
	return NULL;
}

const std::string& Manager::getSwitchName(const switchID_t switchID) {
	if (switches.count(switchID) == 1) {
		return switches.at(switchID)->name;
	}
	return unknownSwitch;
}

