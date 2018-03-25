#include <iostream>
#include <sstream>
#include <unistd.h>

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
using std::stringstream;
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
	if (storage == nullptr) {
		xlog("Unable to create storage handler");
		return;
	}

	//loadDefaultValuesToDB();

	controls[CONTROL_ID_CONSOLE] = new Console(*this, config.getValue("consoleport", 2222));
	controls[CONTROL_ID_WEBSERVER] = new WebServer(*this, config.getValue("webserverport", 80));

	storage->allHardwareParams(hardwareParams);
	for (auto hardwareParam : hardwareParams) {
		hardwareParam.second->manager = this;
		controls[hardwareParam.second->controlID] = new HardwareHandler(*this, hardwareParam.second);
		xlog("Loaded control %i: %s", hardwareParam.first, hardwareParam.second->name.c_str());
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
	while (!locoStopAll()) {
		sleep(1);
	}

	if (storage == nullptr) {
		return;
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
	for (auto control : controls) {
		controlID_t controlID = control.first;
		if (controlID < CONTROL_ID_FIRST_HARDWARE) {
			delete control.second;
			continue;
		}
		if (hardwareParams.count(controlID) != 1) {
			continue;
		}
		HardwareParams* params = hardwareParams.at(controlID);
		if (params == nullptr) {
			continue;
		}
		xlog("Unloading control %i: %s", controlID, params->name.c_str());
		delete control.second;
		hardwareParams.erase(controlID);
		delete params;
	}

	delete storage;
	storage = NULL;
}

/***************************
* Booster                  *
***************************/

void Manager::booster(const managerID_t managerID, const boosterStatus_t status) {
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->booster(managerID, status);
	}
}

/***************************
* Control                  *
***************************/

bool Manager::controlSave(const controlID_t& controlID, const hardwareType_t& hardwareType, const std::string& name, const std::string& ip, string& result) {
	if (controlID != CONTROL_ID_NONE && controlID < CONTROL_ID_FIRST_HARDWARE) {
		result.assign("Invalid controlID");
		return false;
	}
	HardwareParams* params;
	{
		std::lock_guard<std::mutex> Guard(hardwareMutex);
		if (hardwareParams.count(controlID) == 1) {
			params = hardwareParams.at(controlID);
			if (params == nullptr) {
				result.assign("Control does not exist");
				return false;
			}
			params->name = name;
			params->ip = ip;
			// FIXME: reload hardware
		}
		else {
			std::lock_guard<std::mutex> Guard(controlMutex);
			controlID_t newControlID = CONTROL_ID_FIRST_HARDWARE - 1;
			// get next controlID
			for (auto control : controls) {
				if (control.first > newControlID) {
					newControlID = control.first;
				}
			}
			++newControlID;
			// create new control
			params = new HardwareParams(newControlID, hardwareType, name, ip);
			if (params == nullptr) {
				result.assign("Unable to allocate memory for control");
				return false;
			}
			hardwareParams[newControlID] = params;
			controls[newControlID] = new HardwareHandler(*this, params);
		}
	}
	if (storage) {
		storage->hardwareParams(*params);
	}
	return true;
}

bool Manager::controlDelete(controlID_t controlID) {
	HardwareParams* params = nullptr;
	{
		std::lock_guard<std::mutex> Guard(hardwareMutex);
		if (controlID < CONTROL_ID_FIRST_HARDWARE || hardwareParams.count(controlID) != 1) {
			return false;
		}

		if (hardwareParams.count(controlID) != 1) {
			return false;
		}
		params = hardwareParams.at(controlID);
		if (params == nullptr) {
			return false;
		}
	}
	{
		std::lock_guard<std::mutex> Guard(controlMutex);
		if (controls.count(controlID) != 1) {
			return false;
		}
		ManagerInterface* control = controls.at(controlID);
		controls.erase(controlID);
		delete control;
	}

	hardwareParams.erase(controlID);
	delete params;
	if (storage) {
		storage->deleteHardwareParams(controlID);
	}
	return true;
}

HardwareParams* Manager::getHardware(controlID_t controlID) {
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) != 1) {
		return NULL;
	}
	return hardwareParams.at(controlID);
}

unsigned int Manager::controlsOfHardwareType(const hardwareType_t hardwareType) {
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	unsigned int counter = 0;
	for (auto hardwareParam : hardwareParams) {
		if (hardwareParam.second->hardwareType == hardwareType) {
			counter++;
		}
	}
	return counter;
}

bool Manager::hardwareLibraryAdd(const hardwareType_t hardwareType, void* libraryHandle) {
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) == 1) {
		return false;
	}
	hardwareLibraries[hardwareType] = libraryHandle;
	return true;
}

void* Manager::hardwareLibraryGet(const hardwareType_t hardwareType) const {
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) != 1) {
		return nullptr;
	}
	return hardwareLibraries.at(hardwareType);
}

bool Manager::hardwareLibraryRemove(const hardwareType_t hardwareType) {
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) != 1) {
		return false;
	}
	hardwareLibraries.erase(hardwareType);
	return true;
}

const std::map<controlID_t,std::string> Manager::controlListNames() const {
	std::map<controlID_t,std::string> ret;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	for (auto hardware : hardwareParams) {
		ret[hardware.first] = hardware.second->name;
	}
	return ret;
}

const std::map<protocol_t,std::string> Manager::protocolsOfControl(controlID_t controlID) const {
	std::map<protocol_t,std::string> ret;
	std::vector<protocol_t> protocols;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) == 1) {
		std::lock_guard<std::mutex> Guard(controlMutex);
		for (auto control : controls) {
			if (control.second->getManagerID() != MANAGER_ID_HARDWARE) {
				continue;
			}
			const HardwareHandler* hardware = static_cast<const HardwareHandler*>(control.second);
			if (hardware->getControlID() != controlID) {
				continue;
			}
			hardware->getProtocols(protocols);
			for(auto protocol : protocols) {
				ret[protocol] = protocolSymbols[protocol];
			}
			return ret;
		}
	}
	else {
		ret[0] = protocolSymbols[0];
	}
	return ret;
}

/***************************
* Loco                     *
***************************/

datamodel::Loco* Manager::getLoco(const locoID_t locoID) const {
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) != 1) {
		return NULL;
	}
	return locos.at(locoID);
}

const std::string& Manager::getLocoName(const locoID_t locoID) {
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) != 1) {
		return unknownLoco;
	}
	return locos.at(locoID)->name;
}

bool Manager::locoSave(const locoID_t locoID, const string& name, const controlID_t controlID, const protocol_t protocol, const address_t address, string& result) {
	Loco* loco;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		if (locoID != LOCO_NONE && locos.count(locoID)) {
			// update existing loco
			loco = locos.at(locoID);
			if (loco == nullptr) {
				result.assign("Loco does not exist");
				return false;
			}
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
				if (loco.first > newLocoID) {
					newLocoID = loco.first;
				}
			}
			++newLocoID;
			loco = new Loco(this, newLocoID, name, controlID, protocol, address);
			if (loco == nullptr) {
				result.assign("Unable to allocate memory for loco");
				return false;
			}
			// save in map
			locos[newLocoID] = loco;
		}
	}
	// save in db
	if (storage) {
		storage->loco(*loco);
	}
	return true;
}

bool Manager::locoDelete(const locoID_t locoID) {
	Loco* loco = nullptr;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		if (locoID == LOCO_NONE || locos.count(locoID) == 0) {
			return false;
		}

		loco = locos.at(locoID);
		if (loco->isInUse()) {
			return false;
		}

		locos.erase(locoID);
	}

	delete loco;
	if (storage) {
		storage->deleteLoco(locoID);
	}
	return true;
}

bool Manager::locoProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const {
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

void Manager::locoSpeed(const managerID_t managerID, const protocol_t protocol, const address_t address, const speed_t speed) {
	locoID_t locoID = LOCO_NONE;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		for (auto loco : locos) {
			if (loco.second->protocol == protocol && loco.second->address == address) {
				locoID = loco.first;
				break;
			}
		}
	}
	if (locoID == LOCO_NONE) {
		return;
	}
	locoSpeed(managerID, locoID, speed);
}

bool Manager::locoSpeed(const managerID_t managerID, const locoID_t locoID, const speed_t speed) {
	Loco* loco = getLoco(locoID);
	if (loco == nullptr) {
		return false;
	}
	speed_t s = speed;
	if (speed > MAX_SPEED) {
		s = MAX_SPEED;
	}
	xlog("%s (%i) speed is now %i", loco->name.c_str(), locoID, s);
	loco->Speed(s);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->locoSpeed(managerID, locoID, s);
	}
	return true;
}

const speed_t Manager::locoSpeed(const locoID_t locoID) const {
	Loco* loco = getLoco(locoID);
	if (loco == nullptr) {
		return 0;
	}
	return loco->Speed();
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
	xlog("%s (%i) direction is now %i", getLocoName(locoID).c_str(), locoID, direction);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->locoDirection(managerID, locoID, direction);
	}
}

void Manager::locoFunction(const managerID_t managerID, const locoID_t locoID, const function_t function, const bool on) {
	xlog("%s (%i) function %i is now %s", getLocoName(locoID).c_str(), locoID, function, (on ? "on" : "off"));
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->locoFunction(managerID, locoID, function, on);
	}
}

/***************************
* Accessory                *
***************************/

void Manager::accessory(const managerID_t managerID, const accessoryID_t accessoryID, const accessoryState_t state) {
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->accessory(managerID, accessoryID, state);
	}
}

Accessory* Manager::getAccessory(const accessoryID_t accessoryID) {
	std::lock_guard<std::mutex> Guard(accessoryMutex);
	if (accessories.count(accessoryID) != 1) {
		return NULL;
	}
	return accessories.at(accessoryID);
}

const std::string& Manager::getAccessoryName(const accessoryID_t accessoryID) {
	if (accessories.count(accessoryID) != 1) {
		return unknownAccessory;
	}
	return accessories.at(accessoryID)->name;
}

bool Manager::accessorySave(const accessoryID_t accessoryID, const string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryType_t type, const accessoryState_t state, const accessoryTimeout_t timeout, string& result) {
	Accessory* accessory;
	{
		if (!checkPositionFree(posX, posY, posZ, WIDTH_1, HEIGHT_1, ROTATION_0, result)) {
			result.append(" Unable to add or move accessory");
			return false;
		}
		std::lock_guard<std::mutex> Guard(accessoryMutex);
		if (accessoryID != ACCESSORY_NONE && accessories.count(accessoryID)) {
			// update existing accessory
			accessory = accessories.at(accessoryID);
			if (accessory == nullptr) {
				result.assign("Accessory does not exist");
				return false;
			}
			accessory->name = name;
			accessory->posX = posX;
			accessory->posY = posY;
			accessory->posZ = posZ;
			accessory->controlID = controlID;
			accessory->protocol = protocol;
			accessory->address = address;
			accessory->type = type;
			accessory->state = state;
			accessory->timeout = timeout;
		}
		else {
			// create new accessory
			accessoryID_t newAccessoryID = 0;
			// get next accessoryID
			for (auto accessory : accessories) {
				if (accessory.first > newAccessoryID) {
					newAccessoryID = accessory.first;
				}
			}
			++newAccessoryID;
			accessory = new Accessory(newAccessoryID, name, posX, posY, posZ, ROTATION_0, controlID, protocol, address, type, state, timeout);
			if (accessory == nullptr) {
				result.assign("Unable to allocate memory for accessory");
				return false;
			}
			// save in map
			accessories[newAccessoryID] = accessory;
		}
	}
	// save in db
	if (storage) {
		storage->accessory(*accessory);
	}
	return true;
}

bool Manager::accessoryDelete(const accessoryID_t accessoryID) {
	Accessory* accessory = nullptr;
	{
		std::lock_guard<std::mutex> Guard(accessoryMutex);
		if (accessoryID == ACCESSORY_NONE || accessories.count(accessoryID) == 0) {
			return false;
		}

		accessory = accessories.at(accessoryID);
		accessories.erase(accessoryID);
	}

	delete accessory;
	if (storage) {
		storage->deleteAccessory(accessoryID);
	}
	return true;
}

bool Manager::accessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const {
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

/***************************
* Feedback                 *
***************************/

void Manager::feedback(const managerID_t managerID, const feedbackPin_t pin, const feedbackState_t state) {
	Feedback* feedback = getFeedback(pin);
	if (feedback == nullptr) {
		return;
	}
	xlog("Feedback %i is now %s", pin, (state ? "on" : "off"));
	feedback->setState(state);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->feedback(managerID, pin, state);
	}
}

datamodel::Feedback* Manager::getFeedback(feedbackID_t feedbackID) {
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	if (feedbacks.count(feedbackID) != 1) {
		return NULL;
	}
	return feedbacks.at(feedbackID);
}

const std::string& Manager::getFeedbackName(const feedbackID_t feedbackID) {
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	if (feedbacks.count(feedbackID) != 1) {
		return unknownFeedback;
	}
	return feedbacks.at(feedbackID)->name;
}

bool Manager::feedbackSave(const feedbackID_t feedbackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const feedbackPin_t pin, string& result) {
	Feedback* feedback;
	{
		if (!checkPositionFree(posX, posY, posZ, WIDTH_1, HEIGHT_1, ROTATION_0, result)) {
			result.append(" Unable to add or move feedback");
			return false;
		}
		std::lock_guard<std::mutex> Guard(feedbackMutex);
		if (feedbackID != FEEDBACK_NONE && feedbacks.count(feedbackID)) {
			// update existing feedback
			feedback = feedbacks.at(feedbackID);
			if (feedback == nullptr) {
				result.assign("Feedback does not exist");
				return false;
			}
			feedback->name = name;
			feedback->posX = posX;
			feedback->posY = posY;
			feedback->posZ = posZ;
			feedback->controlID = controlID;
			feedback->pin = pin;
		}
		else {
			// create new feedback
			feedbackID_t newFeedbackID = 0;
			// get next feedbackID
			for (auto feedback : feedbacks) {
				if (feedback.first > newFeedbackID) {
					newFeedbackID = feedback.first;
				}
			}
			++newFeedbackID;
			feedback = new Feedback(this, newFeedbackID, name, posX, posY, posZ, controlID, pin);
			if (feedback == nullptr) {
				result.assign("Unable to allocate memory for feedback");
				return false;
			}
			// save in map
			feedbacks[newFeedbackID] = feedback;
		}
	}
	// save in db
	if (storage) {
		storage->feedback(*feedback);
	}
	return feedback;
}

bool Manager::feedbackDelete(const feedbackID_t feedbackID) {
	Feedback* feedback = nullptr;
	{
		std::lock_guard<std::mutex> Guard(feedbackMutex);
		if (feedbackID == FEEDBACK_NONE || feedbacks.count(feedbackID) == 0) {
			return false;
		}

		feedback = feedbacks.at(feedbackID);
		if (feedback == nullptr) {
			return false;
		}

		feedbacks.erase(feedbackID);
	}

	delete feedback;
	if (storage) {
		storage->deleteFeedback(feedbackID);
	}
	return true;
}

/***************************
* Block                    *
***************************/

void Manager::block(const managerID_t managerID, const blockID_t blockID, const blockState_t state) {
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->block(managerID, blockID, state);
	}
}

Block* Manager::getBlock(const blockID_t blockID) {
	std::lock_guard<std::mutex> Guard(blockMutex);
	if (blocks.count(blockID) != 1) {
		return NULL;
	}
	return blocks.at(blockID);
}

const std::string& Manager::getBlockName(const blockID_t blockID) {
	if (blocks.count(blockID) != 1) {
		return unknownBlock;
	}
	return blocks.at(blockID)->name;
}

bool Manager::blockSave(const blockID_t blockID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutRotation_t rotation, string& result) {
	Block* block;
	{
		if (!checkPositionFree(posX, posY, posZ, width, HEIGHT_1, rotation, result)) {
			result.append(" Unable to add or move  block.");
			return false;
		}
		std::lock_guard<std::mutex> Guard(blockMutex);
		if (blockID != BLOCK_NONE && blocks.count(blockID)) {
			// update existing block
			block = blocks.at(blockID);
			if (block == nullptr) {
				result.assign("Block does not exist");
				return false;
			}
			block->name = name;
			block->width = width;
			block->rotation = rotation;
			block->posX = posX;
			block->posY = posY;
			block->posZ = posZ;
		}
		else {
			// create new block
			blockID_t newblockID = 0;
			// get next blockID
			for (auto block : blocks) {
				if (block.first > newblockID) {
					newblockID = block.first;
				}
			}
			++newblockID;
			block = new Block(newblockID, name, posX, posY, posZ, width, rotation);
			if (block == nullptr) {
				result.assign("Unable to allocate memory for block");
				return false;
			}
			// save in map
			blocks[newblockID] = block;
		}
	}
	// save in db
	if (storage) {
		storage->block(*block);
	}
	return true;
}

bool Manager::blockDelete(const blockID_t blockID) {
	Block* block = nullptr;
	{
		std::lock_guard<std::mutex> Guard(blockMutex);
		if (blockID == BLOCK_NONE || blocks.count(blockID) == 0) {
			return false;
		}

		block = blocks.at(blockID);
		if (block == nullptr || block->isInUse()) {
			return false;
		}

		blocks.erase(blockID);
	}

	delete block;
	if (storage) {
		storage->deleteBlock(blockID);
	}
	return true;
}

/***************************
* Switch                   *
***************************/

Switch* Manager::getSwitch(const switchID_t switchID) {
	std::lock_guard<std::mutex> Guard(switchMutex);
	if (switches.count(switchID) != 1) {
		return NULL;
	}
	return switches.at(switchID);
}

const std::string& Manager::getSwitchName(const switchID_t switchID) {
	if (switches.count(switchID) != 1) {
		return unknownSwitch;
	}
	return switches.at(switchID)->name;
}

bool Manager::switchSave(const switchID_t switchID, const string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutRotation_t rotation, const controlID_t controlID, const protocol_t protocol, const address_t address, const switchType_t type, const switchState_t state, const switchTimeout_t timeout, string& result) {
	Switch* mySwitch;
	{
		if (!checkPositionFree(posX, posY, posZ, WIDTH_1, HEIGHT_1, rotation, result)) {
			result.append(" Unable to add or move switch");
			return false;
		}
		std::lock_guard<std::mutex> Guard(switchMutex);
		if (switchID != SWITCH_NONE && switches.count(switchID)) {
			// update existing switch
			mySwitch = switches.at(switchID);
			if (mySwitch == nullptr) {
				result.assign("Block does not exist");
				return false;
			}
			mySwitch->name = name;
			mySwitch->posX = posX;
			mySwitch->posY = posY;
			mySwitch->posZ = posZ;
			mySwitch->rotation = rotation;
			mySwitch->controlID = controlID;
			mySwitch->protocol = protocol;
			mySwitch->address = address;
			mySwitch->type = type;
			mySwitch->state = state;
			mySwitch->timeout = timeout;
		}
		else {
			// create new switch
			switchID_t newswitchID = 0;
			// get next switchID
			for (auto mySwitch : switches) {
				if (mySwitch.first > newswitchID) {
					newswitchID = mySwitch.first;
				}
			}
			++newswitchID;
			mySwitch = new Switch(newswitchID, name, posX, posY, posZ, rotation, controlID, protocol, address, type, state, timeout);
			if (mySwitch == nullptr) {
				result.assign("Unable to allocate memory for switch");
				return false;
			}
			// save in map
			switches[newswitchID] = mySwitch;
		}
	}
	// save in db
	if (storage) {
		storage->saveSwitch(*mySwitch);
	}
	return true;
}

bool Manager::switchDelete(const switchID_t switchID) {
	Switch* mySwitch = nullptr;
	{
		std::lock_guard<std::mutex> Guard(switchMutex);
		if (switchID == SWITCH_NONE || switches.count(switchID) == 0) {
			return false;
		}

		mySwitch = switches.at(switchID);
		switches.erase(switchID);
	}

	delete mySwitch;
	if (storage) {
		storage->deleteSwitch(switchID);
	}
	return true;
}

bool Manager::switchProtocolAddress(const switchID_t switchID, controlID_t& controlID, protocol_t& protocol, address_t& address) const {
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

/***************************
* Street                   *
***************************/

Street* Manager::getStreet(const streetID_t streetID) {
	std::lock_guard<std::mutex> Guard(streetMutex);
	if (streets.count(streetID) != 1) {
		return NULL;
	}
	return streets.at(streetID);
}

const string& Manager::getStreetName(const streetID_t streetID) {
	if (streets.count(streetID) != 1) {
		return unknownStreet;
	}
	return streets.at(streetID)->name;
}

bool Manager::streetSave(const streetID_t streetID, const std::string& name, const blockID_t fromBlock, const direction_t fromDirection, const blockID_t toBlock, const direction_t toDirection, const feedbackID_t feedbackID, string& result) {
	Street* street;
	{
		std::lock_guard<std::mutex> Guard(streetMutex);
		if (streetID != STREET_NONE && streets.count(streetID)) {
			// update existing street
			street = streets.at(streetID);
			if (street == nullptr) {
				result.assign("Street does not exist");
				return false;
			}
			street->name = name;
			street->fromBlock = fromBlock;
			street->fromDirection = fromDirection;
			street->toBlock = toBlock;
			street->toDirection = toDirection;
			street->feedbackIDStop = feedbackID;
		}
		else {
			// create new street
			streetID_t newStreetID = 0;
			// get next streetID
			for (auto street : streets) {
				if (street.first > newStreetID) {
					newStreetID = street.first;
				}
			}
			++newStreetID;
			street = new Street(this, newStreetID, name, fromBlock, fromDirection, toBlock, toDirection, feedbackID);
			if (street == nullptr) {
				result.assign("Unable to allocate memory for street");
				return false;
			}
			// save in map
			streets[newStreetID] = street;
		}
	}
	// save in db
	if (storage) {
		storage->street(*street);
	}
	return true;
}

bool Manager::streetDelete(const streetID_t streetID) {
	Street* street = nullptr;
	{
		std::lock_guard<std::mutex> Guard(streetMutex);
		if (streetID == STREET_NONE || streets.count(streetID) == 0) {
			return false;
		}

		street = streets.at(streetID);
		streets.erase(streetID);
	}

	delete street;
	if (storage) {
		storage->deleteStreet(streetID);
	}
	return true;
}

/***************************
* Automode                 *
***************************/

bool Manager::locoIntoBlock(const locoID_t locoID, const blockID_t blockID) {
	Block* block = getBlock(blockID);
	if (block == nullptr) {
		return false;
	}

	Loco* loco = getLoco(locoID);
	if (loco == nullptr) {
		return false;
	}

	bool reserved = block->reserve(locoID);
	if (!reserved) {
		return false;
	}

	reserved = loco->toBlock(blockID);
	if (reserved == false) {
		block->release(locoID);
		return false;
	}

	reserved = block->lock(locoID);
	if (reserved == false) {
		loco->releaseBlock();
		block->release(locoID);
		return false;
	}

	xlog("%s (%i) is now in block %s (%i)", loco->name.c_str(), loco->objectID, block->name.c_str(), block->objectID);

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->locoIntoBlock(locoID, blockID);
	}
	return true;
}

bool Manager::locoStreet(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) {
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->locoStreet(locoID, streetID, blockID);
	}
	return true;
}
bool Manager::locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const blockID_t blockID) {
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->locoDestinationReached(locoID, streetID, blockID);
	}
	return true;
}

bool Manager::locoStart(const locoID_t locoID) {
	Loco* loco = getLoco(locoID);
	if (loco == nullptr) {
		return false;
	}
	bool ret = loco->start();
	if (ret == false) {
		return false;
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->locoStart(locoID);
	}
	return true;
}

bool Manager::locoStop(const locoID_t locoID) {
	Loco* loco = getLoco(locoID);
	if (loco == nullptr) {
		return false;
	}
	bool ret = loco->stop();
	if (ret == false) {
		return false;
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls) {
		control.second->locoStop(locoID);
	}
	return true;
}

bool Manager::locoStartAll() {
	for (auto loco : locos) {
		bool ret = loco.second->start();
		if (ret == false) {
			continue;
		}
		{
			std::lock_guard<std::mutex> Guard(controlMutex);
			for (auto control : controls) {
				control.second->locoStart(loco.first);
			}
		}
	}
	return true;
}

bool Manager::locoStopAll() {
	bool ret1 = true;
	for (auto loco : locos) {
		if (!loco.second->isInUse()) {
			continue;
		}
		bool ret2 = loco.second->stop();
		ret1 &= ret2;
		if (ret2 == false) {
			continue;
		}
		{
			std::lock_guard<std::mutex> Guard(controlMutex);
			for (auto control : controls) {
				control.second->locoStop(loco.first);
			}
		}
	}
	return ret1;
}

/***************************
* Default Values           *
***************************/

void Manager::loadDefaultValuesToDB() {
	HardwareParams newHardwareParams1(1, 1, "Virtuelle Zentrale", "");
	storage->hardwareParams(newHardwareParams1);

	HardwareParams newHardwareParams2(2, 2, "CS2 Zentrale", "192.168.0.190");
	storage->hardwareParams(newHardwareParams2);

	Loco newloco1(this, 1, "Re 460 Teddy", 1, PROTOCOL_DCC, 1119);
	storage->loco(newloco1);

	Loco newloco2(this, 2, "ICN", 1, PROTOCOL_DCC, 1118);
	storage->loco(newloco2);

	Accessory newAccessory1(1, "Schalter 1", ROTATION_0, 3, 5, 0, 1, PROTOCOL_DCC, 1, 1, ACCESSORY_STATE_ON, 200);
	storage->accessory(newAccessory1);

	Accessory newAccessory2(2, "Schalter 2", ROTATION_0, 3, 6, 0, 1, PROTOCOL_DCC, 2, 1, ACCESSORY_STATE_OFF, 200);
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

	Switch newSwitch1(1, "Weiche Einfahrt", 2, 5, 0, 1, PROTOCOL_DCC, 3, SWITCH_LEFT, SWITCH_TURNOUT, ROTATION_90, 200);
	storage->saveSwitch(newSwitch1);

	Switch newSwitch2(2, "Weiche Ausfahrt", 2, 6, 0, 1, PROTOCOL_DCC, 4, SWITCH_RIGHT, SWITCH_STRAIGHT, ROTATION_0, 200);
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

/***************************
* Layout                   *
***************************/

bool Manager::mapPosition(const layoutPosition_t posX,
	const layoutPosition_t posY,
	const layoutItemSize_t width,
	const layoutItemSize_t height,
	const layoutRotation_t rotation,
	layoutPosition_t& x,
	layoutPosition_t& y,
	layoutItemSize_t& w,
	layoutItemSize_t& h) {

	switch (rotation) {
		case ROTATION_0:
			x = posX;
			y = posY;
			w = width;
			h = height;
			return true;
		case ROTATION_90:
			if (posX < height) {
				return false;
			}
			x = posX + 1 - height;
			y = posY;
			w = height;
			h = width;
			return true;
		case ROTATION_180:
			if (posX < width || posY < height) {
				return false;
			}
			x = posX + 1 - width;
			y = posY + 1 - height;
			w = width;
			h = height;
			return true;
		case ROTATION_270:
			if (posY < width) {
				return false;
			}
			x = posX;
			y = posY + 1 - width;
			w = height;
			h = width;
			return true;
		default:
			return false;
	}
}

bool Manager::checkPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation, string& result) {
	if (width == 0 || height == 0) {
		result.assign("Width or height is zero.");
		return false;
	}
	layoutPosition_t x;
	layoutPosition_t y;
	layoutPosition_t z = posZ;
	layoutItemSize_t w;
	layoutItemSize_t h;
	bool ret = mapPosition(posX, posY, width, height, rotation, x, y, w, h);
	if (ret == false) {
		return false;
	}
	for(layoutPosition_t ix = x; ix < x + w; ix++) {
		for(layoutPosition_t iy = y; iy < y + h; iy++) {
			bool ret = checkAccessoryPositionFree(ix, iy, z, result);
			if (ret == false) {
				return false;
			}
			ret = checkBlockPositionFree(ix, iy, z, result);
			if (ret == false) {
				return false;
			}
			ret = checkFeedbackPositionFree(ix, iy, z, result);
			if (ret == false) {
				return false;
			}
			ret = checkSwitchPositionFree(ix, iy, z, result);
			if (ret == false) {
				return false;
			}
		}
	}
	return true;
}

bool Manager::checkAccessoryPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, string& result) {
	std::lock_guard<std::mutex> Guard(accessoryMutex);
	for (auto accessory : accessories) {
		if (accessory.second->posX == posX && accessory.second->posY == posY && accessory.second->posZ == posZ) {
			stringstream status;
			status << "Position " << static_cast<int>(posX) << "/" << static_cast<int>(posY) << "/" << static_cast<int>(posZ) << " is already used by accessory \"" << accessory.second->name << "\".";
			result.assign(status.str());
			return false;
		}
	}
	return true;
}

bool Manager::checkBlockPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, string& result) {
	std::lock_guard<std::mutex> Guard(blockMutex);
	for (auto block : blocks) {
		Block* b = block.second;
		if (b->posZ != posZ) {
			continue;
		}
		layoutPosition_t x;
		layoutPosition_t y;
		layoutItemSize_t w;
		layoutItemSize_t h;
		bool ret = mapPosition(b->posX, b->posY, b->width, b->height, b->rotation, x, y, w, h);
		if (ret == false) {
			stringstream status;
			status << "Block \"" << block.second->name << "\" has an invalid position.";
			result.assign(status.str());
			return false;
		}
		for(layoutPosition_t ix = x; ix < x + w; ix++) {
			for(layoutPosition_t iy = y; iy < y + h; iy++) {
				if (ix == posX && iy == posY) {
					stringstream status;
					status << "Position " << static_cast<int>(posX) << "/" << static_cast<int>(posY) << "/" << static_cast<int>(posZ) << " is already used by block \"" << block.second->name << "\".";
					result.assign(status.str());
					return false;
				}
			}
		}
	}
	return true;
}

bool Manager::checkFeedbackPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, string& result) {
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	for (auto feedback : feedbacks) {
		if (feedback.second->posX == posX && feedback.second->posY == posY && feedback.second->posZ == posZ) {
			stringstream status;
			status << "Position " << static_cast<int>(posX) << "/" << static_cast<int>(posY) << "/" << static_cast<int>(posZ) << " is already used by feedback \"" << feedback.second->name << "\".";
			result.assign(status.str());
			return false;
		}
	}
	return true;
}

bool Manager::checkSwitchPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, string& result) {
	std::lock_guard<std::mutex> Guard(switchMutex);
	for (auto mySwitch : switches) {
		if (mySwitch.second->posX == posX && mySwitch.second->posY == posY && mySwitch.second->posZ == posZ) {
			stringstream status;
			status << "Position " << static_cast<int>(posX) << "/" << static_cast<int>(posY) << "/" << static_cast<int>(posZ) << " is already used by switch \"" << mySwitch.second->name << "\".";
			result.assign(status.str());
			return false;
		}
	}
	return true;
}

