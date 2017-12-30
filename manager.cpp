#include <iostream>
#include <sstream>

#include "console/console.h"
#include "hardware/hardware_handler.h"
#include "hardware/hardware_params.h"
#include "manager.h"
#include "railcontrol.h"
#include "util.h"
#include "webserver/webserver.h"

using console::Console;
using datamodel::Accessory;
using datamodel::Block;
using datamodel::Feedback;
using datamodel::Loco;
using datamodel::Street;
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
	unknownSwitch("Unknown Switch"),
	unknownStreet("Unknown Street") {

	StorageParams storageParams;
	storageParams.module = config.getValue("dbengine", "sqlite");
	storageParams.filename = config.getValue("dbfilename", "/tmp/railcontrol.db");
	storage = new StorageHandler(this, storageParams);

	loadDefaultValuesToDB();

	controllers.push_back(new Console(*this, config.getValue("consoleport", 2222)));
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
		xlog("Loaded accessory %i: %s", accessory.second->objectID, accessory.second->name.c_str());
	}

	storage->allFeedbacks(feedbacks);
	for (auto feedback : feedbacks) {
		xlog("Loaded feedback %i: %s", feedback.second->objectID, feedback.second->name.c_str());
	}

	storage->allBlocks(blocks);
	for (auto block : blocks) {
		xlog("Loaded block %i: %s", block.second->objectID, block.second->name.c_str());
	}

	storage->allSwitches(switches);
	for (auto mySwitch : switches) {
		xlog("Loaded switch %i: %s", mySwitch.second->objectID, mySwitch.second->name.c_str());
	}

	storage->allStreets(streets);
	for (auto street : streets) {
		xlog("Loaded street %i: %s", street.second->objectID, street.second->name.c_str());
	}
	// FIXME: load locos into blocks
}

Manager::~Manager() {
	stopLoco(1);
	stopAllLocos();

	for (auto controller : controllers) {
		delete controller;
	}
	for (auto hardwareParam : hardwareParams) {
		xlog("Unloaded controller %i: %s", hardwareParam.first, hardwareParam.second->name.c_str());
		delete hardwareParam.second;
	}
	for (auto street : streets) {
		xlog("Saving street %i: %s", street.second->objectID, street.second->name.c_str());
		storage->street(*(street.second));
		delete street.second;
	}
	for (auto mySwitch : switches) {
		xlog("Saving switch %i: %s", mySwitch.second->objectID, mySwitch.second->name.c_str());
		storage->saveSwitch(*(mySwitch.second));
		delete mySwitch.second;
	}
	for (auto accessory : accessories) {
		xlog("Saving accessory %i: %s", accessory.second->objectID, accessory.second->name.c_str());
		storage->accessory(*(accessory.second));
		delete accessory.second;
	}
	for (auto  feedback : feedbacks) {
		xlog("Saving feedback %i: %s", feedback.second->objectID, feedback.second->name.c_str());
		storage->feedback(*(feedback.second));
		delete feedback.second;
	}
	for (auto block : blocks) {
		xlog("Saving block %i: %s", block.second->objectID, block.second->name.c_str());
		storage->block(*(block.second));
		delete block.second;
	}
	for (auto loco : locos) {
		xlog("Saving loco %i: %s", loco.second->objectID, loco.second->name.c_str());
		storage->loco(*(loco.second));
		delete loco.second;
	}
	delete storage;
	storage = NULL;
}

void Manager::loadDefaultValuesToDB() {
	//HardwareParams newHardwareParams(1, 1, "Virtuelle Zentrale", "");
	HardwareParams newHardwareParams(1, 2, "CS2 Zentrale", "192.168.0.190");
	storage->hardwareParams(newHardwareParams);

	Loco newloco1(this, 1, "Re 460 Teddy", 1, PROTOCOL_DCC, 1119);
	storage->loco(newloco1);

	Loco newloco2(this, 2, "ICN", 1, PROTOCOL_DCC, 1118);
	storage->loco(newloco2);

	Accessory newAccessory1(1, "Schalter 1", 1, PROTOCOL_DCC, 1, 1, ACCESSORY_STATE_ON, 200, 3, 5, 0);
	storage->accessory(newAccessory1);

	Accessory newAccessory2(2, "Schalter 2", 1, PROTOCOL_DCC, 2, 1, ACCESSORY_STATE_OFF, 200, 3, 6, 0);
	storage->accessory(newAccessory2);

	Feedback newFeedback1(this, 1, "Rückmelder Bahnhof 1", 1, 1, 4, 5, 0);
	storage->feedback(newFeedback1);

	Feedback newFeedback2(this, 2, "Rückmelder Bahnhof 2", 1, 2, 4, 6, 0);
	storage->feedback(newFeedback2);

	Feedback newFeedback3(this, 3, "Rückmelder Ausfahrt", 1, 3, 4, 6, 0);
	storage->feedback(newFeedback3);

	Feedback newFeedback4(this, 4, "Rückmelder Strecke", 1, 4, 4, 6, 0);
	storage->feedback(newFeedback4);

	Feedback newFeedback5(this, 5, "Rückmelder Einfahrt", 1, 5, 4, 6, 0);
	storage->feedback(newFeedback5);

	Block newBlock1(1, "Block Bahnhof 1", 4, ROTATION_0, 5, 5, 0);
	storage->block(newBlock1);

	Block newBlock2(2, "Block Bahnhof 2", 4, ROTATION_90, 5, 6, 0);
	storage->block(newBlock2);

	Block newBlock3(3, "Block Ausfahrt", 4, ROTATION_90, 5, 6, 0);
	storage->block(newBlock3);

	Block newBlock4(4, "Block Einfahrt", 4, ROTATION_90, 5, 6, 0);
	storage->block(newBlock4);

	Block newBlock5(5, "Block Strecke", 4, ROTATION_90, 5, 6, 0);
	storage->block(newBlock5);

	Switch newSwitch1(1, "Weiche Einfahrt", 1, PROTOCOL_DCC, 3, SWITCH_LEFT, SWITCH_TURNOUT, ROTATION_90, 2, 5, 0);
	storage->saveSwitch(newSwitch1);

	Switch newSwitch2(2, "Weiche Ausfahrt", 1, PROTOCOL_DCC, 4, SWITCH_RIGHT, SWITCH_STRAIGHT, ROTATION_0, 2, 6, 0);
	storage->saveSwitch(newSwitch2);

	Street newStreet1(this, 1, "Fahrstrasse Ausfahrt 1", 1, false, 3, false, 3);
	storage->street(newStreet1);

	Street newStreet2(this, 2, "Fahrstrasse Ausfahrt 2", 2, false, 3, false, 3);
	storage->street(newStreet2);

	Street newStreet3(this, 3, "Fahrstrasse Auf Strecke", 3, false, 4, false, 4);
	storage->street(newStreet3);

	Street newStreet4(this, 4, "Fahrstrasse Von Strecke", 4, false, 5, false, 5);
	storage->street(newStreet4);

	Street newStreet5(this, 5, "Fahrstrasse Einfahrt 1", 5, false, 1, false, 1);
	storage->street(newStreet5);

	Street newStreet6(this, 6, "Fahrstrasse Einfahrt 2", 5, false, 2, false, 2);
	storage->street(newStreet6);
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
	locoID_t locoID = 0;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		for (auto loco : locos) {
			if (loco.second->protocol == protocol && loco.second->address == address) {
				locoID = loco.first;
				break;
			}
		}
	}
	if (locoID) {
		locoSpeed(managerID, locoID, speed);
	}
}

void Manager::locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) {
	xlog("Setting speed of loco \"%s\" (%i) to speed %i", getLocoName(locoID).c_str(), locoID, speed);
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
	xlog("Setting direction of loco \"%s\" (%i) to %i", getLocoName(locoID).c_str(), locoID, direction);
	for (auto control : controllers) {
		control->locoDirection(managerID, locoID, direction);
	}
}

void Manager::locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) {
	xlog("Setting function %i of loco \"%s\" (%i) to %i", function, getLocoName(locoID).c_str(), locoID, on);
	for (auto control : controllers) {
		control->locoFunction(managerID, locoID, function, on);
	}
}

const std::map<locoID_t,datamodel::Loco*>& Manager::locoList() const {
	return locos;
}

datamodel::Loco* Manager::getLoco(locoID_t locoID) {
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) == 1) {
		return locos.at(locoID);
	}
	return NULL;
}

void Manager::locoSave(const locoID_t locoID, const string& name, controlID_t& controlID, protocol_t& protocol, address_t& address) {
	Loco* loco;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
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
			loco = new Loco(this, newLocoID, name, controlID, protocol, address);
			// save in map
			locos[newLocoID] = loco;
		}
	}
	// save in db
	storage->loco(*loco);
}

void Manager::feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) {
	Feedback* feedback = getFeedback(pin);
	if (feedback) {
		xlog("Setting feedback %i to %s", pin, (state ? "on" : "off"));
		feedback->setState(state);
	}
	for (auto control : controllers) {
		control->feedback(managerID, pin, state);
	}
}

datamodel::Feedback* Manager::getFeedback(feedbackID_t feedbackID) {
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	if (feedbacks.count(feedbackID) == 1) {
		return feedbacks.at(feedbackID);
	}
	return NULL;
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

Street* Manager::getStreet(const streetID_t streetID) {
	std::lock_guard<std::mutex> Guard(streetMutex);
	if (streets.count(streetID) == 1) {
		return streets.at(streetID);
	}
	return NULL;
}

bool Manager::locoIntoBlock(const locoID_t locoID, const blockID_t blockID) {
	Block* block = getBlock(blockID);
	if (!block) return false;

	Loco* loco = getLoco(locoID);
	if (!loco) return false;

	bool reserved = block->reserve(locoID);
	if (!reserved) return false;

	reserved = loco->toBlock(blockID);
	if (!reserved) {
		block->release(locoID);
		return false;
	}

	reserved = block->lock(locoID);
	if (!reserved) {
		loco->releaseBlock();
		block->release(locoID);
		return false;
	}

	std::stringstream ss;
	ss << "Loco \"" << loco->name << "\" is now in block \"" << block->name << "\"";
	string s(ss.str());
	xlog(s.c_str());
	return true;
}

bool Manager::startLoco(const locoID_t locoID) {
	Loco* loco = getLoco(locoID);
	if (loco) {
		return loco->start();
	}
	return false;
}

bool Manager::stopLoco(const locoID_t locoID) {
	Loco* loco = getLoco(locoID);
	if (loco) {
		return loco->stop();
	}
	return false;
}

bool Manager::startAllLocos() {
	return false;
}

bool Manager::stopAllLocos() {
	return false;
}
