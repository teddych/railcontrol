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
	autoMode(false),
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

	//loadDefaultValuesToDB();

	controllers.push_back(new WebServer(*this, config.getValue("webserverport", 80)));

	storage->allHardwareParams(hardwareParams);
	for (auto hardwareParam : hardwareParams) {
		hardwareParam.second->manager = this;
		controllers.push_back(new HardwareHandler(*this, hardwareParam.second));
		xlog("Loaded controller %i: %s", hardwareParam.first, hardwareParam.second->name.c_str());
	}

	storage->allLocos(locos);
	for (auto loco : locos) {
		xlog("Loaded loco %i: %s", loco.second->objectID, loco.second->name.c_str());
	}

	storage->allAccessories(accessories);
	for (auto accessory : accessories) {
		xlog("Loaded accessory %i: %s", accessory.second->accessoryID, accessory.second->name.c_str());
	}

	storage->allFeedbacks(feedbacks);
	for (auto feedback : feedbacks) {
		xlog("Loaded feedback %i: %s", feedback.second->feedbackID, feedback.second->name.c_str());
	}

	storage->allBlocks(blocks);
	for (auto block : blocks) {
		xlog("Loaded block %i: %s", block.second->blockID, block.second->name.c_str());
	}

	storage->allSwitches(switches);
	for (auto mySwitch : switches) {
		xlog("Loaded switch %i: %s", mySwitch.second->switchID, mySwitch.second->name.c_str());
	}
}

Manager::~Manager() {
	for (auto control : controllers) {
		delete control;
	}
	for (auto hardwareParam : hardwareParams) {
		xlog("Unloaded controller %i: %s", hardwareParam.first, hardwareParam.second->name.c_str());
		delete hardwareParam.second;
	}
	for (auto loco : locos) {
		xlog("Saving loco %i: %s", loco.second->objectID, loco.second->name.c_str());
		storage->loco(*(loco.second));
		delete loco.second;
	}
	for (auto accessory : accessories) {
		xlog("Saving accessory %i: %s", accessory.second->accessoryID, accessory.second->name.c_str());
		storage->accessory(*(accessory.second));
		delete accessory.second;
	}
	for (auto  feedback : feedbacks) {
		xlog("Saving feedback %i: %s", feedback.second->feedbackID, feedback.second->name.c_str());
		storage->feedback(*(feedback.second));
		delete feedback.second;
	}
	for (auto block : blocks) {
		xlog("Saving block %i: %s", block.second->blockID, block.second->name.c_str());
		storage->block(*(block.second));
		delete block.second;
	}
	for (auto mySwitch : switches) {
		xlog("Saving wwitch %i: %s", mySwitch.second->switchID, mySwitch.second->name.c_str());
		storage->saveSwitch(*(mySwitch.second));
		delete mySwitch.second;
	}
	delete storage;
	storage = NULL;
}

void Manager::loadDefaultValuesToDB() {
	HardwareParams newHardwareParams(1, 1, "Virtuelle Zentrale", "");
	storage->hardwareParams(newHardwareParams);

	Loco newloco1(1, "Re 460 Teddy", 1, PROTOCOL_DCC, 1119);
	storage->loco(newloco1);

	Loco newloco2(2, "ICN", 1, PROTOCOL_DCC, 1118);
	storage->loco(newloco2);

	Accessory newAccessory1(1, "Schalter 1", 1, PROTOCOL_DCC, 1, 1, ACCESSORY_STATE_ON, 200, 3, 5, 0);
	storage->accessory(newAccessory1);

	Accessory newAccessory2(2, "Schalter 2", 1, PROTOCOL_DCC, 2, 1, ACCESSORY_STATE_OFF, 200, 3, 6, 0);
	storage->accessory(newAccessory2);

	Feedback newFeedback1(1, "Rückmelder Einfahrt links", 1, 1, 4, 5, 0);
	storage->feedback(newFeedback1);

	Feedback newFeedback2(2, "Rückmelder Einfahrt rechts", 1, 2, 4, 6, 0);
	storage->feedback(newFeedback2);

	Block newBlock1(1, "Block 1", 4, ROTATION_0, 5, 5, 0);
	storage->block(newBlock1);

	Block newBlock2(2, "Block 2", 4, ROTATION_90, 5, 6, 0);
	storage->block(newBlock2);

	Switch newSwitch1(1, "Weiche 1", 1, PROTOCOL_DCC, 3, SWITCH_LEFT, SWITCH_TURNOUT, ROTATION_90, 2, 5, 0);
	storage->saveSwitch(newSwitch1);

	Switch newSwitch2(2, "Weiche 2", 1, PROTOCOL_DCC, 4, SWITCH_RIGHT, SWITCH_STRAIGHT, ROTATION_0, 2, 6, 0);
	storage->saveSwitch(newSwitch2);
}

void Manager::booster(const managerID_t managerID, const boosterStatus_t status) {
  for (auto control : controllers) {
		control->booster(managerID, status);
	}
}

void Manager::saveHardware(const controlID_t& controlID, const hardwareID_t& hardwareID, const std::string& name, const std::string& ip) {
	HardwareParams* params;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
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
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) == 1) {
		//vector<>::iterator control =
		// FIXME: deleteHardware not implemented
		// FIXME: check if there are no elements on this Hardware
	}
}

const std::map<controlID_t,std::string> Manager::controlList() const {
	std::map<controlID_t,std::string> ret;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	for (auto hardware : hardwareParams) {
		ret[hardware.first] = hardware.second->name;
	}
	return ret;
}

/*
const hardwareID_t Manager::hardwareOfControl(controlID_t controlID) const {
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) == 1) {
		const HardwareParams* params = hardwareParams.at(controlID);
		return params->hardwareID;
	}
	return HARDWARE_ID_NONE;
}
*/

const std::map<protocol_t,std::string> Manager::protocolsOfControl(controlID_t controlID) const {
	std::map<protocol_t,std::string> ret;
	std::vector<protocol_t> protocols;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) == 1) {
		const ManagerInterface* control = controllers.at(controlID);
		managerID_t managerID = control->getManagerID();
		if (managerID == MANAGER_ID_HARDWARE) {
			const HardwareHandler* hardware = static_cast<const HardwareHandler*>(control);
			hardware->getProtocols(protocols);
			for(auto protocol : protocols) {
				ret[protocol] = protocolSymbols[protocol];
			}
		}
	}
	else {
		ret[0] = protocolSymbols[0];
	}
	return ret;
}

HardwareParams* Manager::getHardware(controlID_t controlID) {
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) == 1) {
		return hardwareParams.at(controlID);
	}
	return NULL;
}

bool Manager::getProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const {
	std::lock_guard<std::mutex> Guard(locoMutex);
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
	std::lock_guard<std::mutex> Guard(locoMutex);
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
	std::lock_guard<std::mutex> Guard(locoMutex);
	for (auto loco : locos) {
		if (loco.second->protocol == protocol && loco.second->address == address) {
			locoSpeed(managerID, loco.first, speed);
			return;
		}
	}
}

void Manager::locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) {
	std::lock_guard<std::mutex> Guard(locoMutex);
  for (auto control : controllers) {
    control->locoSpeed(managerID, locoID, speed);
  }
}

void Manager::locoDirection(const managerID_t managerID, const protocol_t protocol, const address_t address, const direction_t direction) {
	std::lock_guard<std::mutex> Guard(locoMutex);
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

const datamodel::Loco* Manager::getLoco(locoID_t locoID) const {
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) == 1) {
		return locos.at(locoID);
	}
	return NULL;
}

void Manager::locoSave(const locoID_t locoID, const string& name, controlID_t& controlID, protocol_t& protocol, address_t& address) {
	std::lock_guard<std::mutex> Guard(locoMutex);
	Loco* loco;
	if (locoID && locos.count(locoID)) {
		// update existing loco
		loco = locos.at(locoID);
		loco->name = name;
		loco->controlID = controlID;
		loco->protocol = protocol;
		loco->address = address;
	}
	else {
		// create new loco
		locoID_t newLocoID = 0;
		// get next locoID
		for (auto loco : locos) {
			if (loco.first > locoID) {
				newLocoID = loco.first;
			}
		}
		++newLocoID;
		loco = new Loco(newLocoID, name, controlID, protocol, address);
		// save in map
		locos[newLocoID] = loco;
	}
	// save in db
	storage->loco(*loco);
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

