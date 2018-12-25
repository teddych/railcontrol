#include <iostream>
#include <sstream>
#include <unistd.h>

#include "console/console.h"
#include "datamodel/layout_item.h"
#include "DelayedCall.h"
#include "hardware/HardwareHandler.h"
#include "hardware/HardwareParams.h"
#include "manager.h"
#include "railcontrol.h"
#include "util.h"
#include "webserver/webserver.h"

using console::Console;
using datamodel::Accessory;
using datamodel::Track;
using datamodel::Feedback;
using datamodel::LayoutItem;
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

Manager::Manager(Config& config)
:	storage(nullptr),
 	delayedCall(new DelayedCall(*this)),
	unknownControl("Unknown Control"),
	unknownLoco("Unknown Loco"),
	unknownAccessory("Unknown Accessory"),
	unknownFeedback("Unknown Feedback"),
	unknownTrack("Unknown Track"),
	unknownSwitch("Unknown Switch"),
	unknownStreet("Unknown Street")
{
	StorageParams storageParams;
	storageParams.module = config.getValue("dbengine", "sqlite");
	storageParams.filename = config.getValue("dbfilename", "/tmp/railcontrol.db");
	storage = new StorageHandler(this, storageParams);
	if (storage == nullptr)
	{
		xlog("Unable to create storage handler");
		return;
	}

	//loadDefaultValuesToDB();

	controls[ControlIdConsole] = new Console(*this, config.getValue("consoleport", 2222));
	controls[ControlIdWebserver] = new WebServer(*this, config.getValue("webserverport", 80));

	storage->allHardwareParams(hardwareParams);
	for (auto hardwareParam : hardwareParams)
	{
		hardwareParam.second->manager = this;
		controls[hardwareParam.second->controlID] = new HardwareHandler(*this, hardwareParam.second);
		xlog("Loaded control %i: %s", hardwareParam.first, hardwareParam.second->name.c_str());
	}

	storage->allLocos(locos);
	for (auto loco : locos)
	{
		xlog("Loaded loco %i: %s", loco.second->objectID, loco.second->name.c_str());
	}

	storage->allAccessories(accessories);
	for (auto accessory : accessories)
	{
		xlog("Loaded accessory %i: %s", accessory.second->objectID, accessory.second->name.c_str());
	}

	storage->allFeedbacks(feedbacks);
	for (auto feedback : feedbacks)
	{
		xlog("Loaded feedback %i: %s", feedback.second->objectID, feedback.second->name.c_str());
	}

	storage->allTracks(tracks);
	for (auto track : tracks)
	{
		xlog("Loaded track %i: %s", track.second->objectID, track.second->name.c_str());
	}

	storage->allSwitches(switches);
	for (auto mySwitch : switches)
	{
		xlog("Loaded switch %i: %s", mySwitch.second->objectID, mySwitch.second->name.c_str());
	}

	storage->allStreets(streets);
	for (auto street : streets)
	{
		xlog("Loaded street %i: %s", street.second->objectID, street.second->name.c_str());
	}
	// FIXME: load locos into tracks
}

Manager::~Manager()
{
	while (!locoStopAll())
	{
		sleep(1);
	}

	if (storage == nullptr)
	{
		return;
	}

	for (auto street : streets)
	{
		xlog("Saving street %i: %s", street.second->objectID, street.second->name.c_str());
		storage->street(*(street.second));
		delete street.second;
	}
	for (auto mySwitch : switches)
	{
		xlog("Saving switch %i: %s", mySwitch.second->objectID, mySwitch.second->name.c_str());
		storage->saveSwitch(*(mySwitch.second));
		delete mySwitch.second;
	}
	for (auto accessory : accessories)
	{
		xlog("Saving accessory %i: %s", accessory.second->objectID, accessory.second->name.c_str());
		storage->accessory(*(accessory.second));
		delete accessory.second;
	}
	for (auto  feedback : feedbacks)
	{
		xlog("Saving feedback %i: %s", feedback.second->objectID, feedback.second->name.c_str());
		storage->feedback(*(feedback.second));
		delete feedback.second;
	}
	for (auto track : tracks)
	{
		xlog("Saving track %i: %s", track.second->objectID, track.second->name.c_str());
		storage->track(*(track.second));
		delete track.second;
	}
	for (auto loco : locos)
	{
		xlog("Saving loco %i: %s", loco.second->objectID, loco.second->name.c_str());
		storage->loco(*(loco.second));
		delete loco.second;
	}
	for (auto control : controls)
	{
		controlID_t controlID = control.first;
		if (controlID < ControlIdFirstHardware)
		{
			delete control.second;
			continue;
		}
		if (hardwareParams.count(controlID) != 1)
		{
			continue;
		}
		HardwareParams* params = hardwareParams.at(controlID);
		if (params == nullptr)
		{
			continue;
		}
		xlog("Unloading control %i: %s", controlID, params->name.c_str());
		delete control.second;
		hardwareParams.erase(controlID);
		delete params;
	}

	delete delayedCall;
	delayedCall = nullptr;

	delete storage;
	storage = nullptr;
}

/***************************
* Booster                  *
***************************/

void Manager::booster(const controlType_t managerID, const boosterStatus_t status)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->booster(managerID, status);
	}
}

/***************************
* Control                  *
***************************/

const std::map<hardwareType_t,string> Manager::hardwareListNames()
{
	std::map<hardwareType_t,string> hardwareList;
	hardwareList[HardwareTypeM6051] = "Märklin Interface 6051/6051";
	hardwareList[HardwareTypeCS2] = "Märklin Control Station 2 (CS2)";
	hardwareList[HardwareTypeVirtual] = "Virtual Command Station (no Hardware)";
	return hardwareList;
}

bool Manager::controlSave(const controlID_t& controlID, const hardwareType_t& hardwareType, const std::string& name, const std::string& arg1, string& result)
{
	if (controlID != ControlIdNone && controlID < ControlIdFirstHardware)
	{
		result.assign("Invalid controlID");
		return false;
	}
	HardwareParams* params;
	{
		std::lock_guard<std::mutex> Guard(hardwareMutex);
		if (hardwareParams.count(controlID) == 1)
		{
			params = hardwareParams.at(controlID);
			if (params == nullptr)
			{
				result.assign("Control does not exist");
				return false;
			}
			params->name = name;
			params->hardwareType = hardwareType;
			params->arg1 = arg1;
			// FIXME: reload hardware
		}
		else
		{
			std::lock_guard<std::mutex> Guard(controlMutex);
			controlID_t newControlID = ControlIdFirstHardware - 1;
			// get next controlID
			for (auto control : controls)
			{
				if (control.first > newControlID)
				{
					newControlID = control.first;
				}
			}
			++newControlID;
			// create new control
			params = new HardwareParams(newControlID, hardwareType, name, arg1);
			if (params == nullptr)
			{
				result.assign("Unable to allocate memory for control");
				return false;
			}
			hardwareParams[newControlID] = params;
			controls[newControlID] = new HardwareHandler(*this, params);
		}
	}
	if (storage)
	{
		storage->hardwareParams(*params);
	}
	return true;
}

bool Manager::controlDelete(controlID_t controlID)
{
	HardwareParams* params = nullptr;
	{
		std::lock_guard<std::mutex> Guard(hardwareMutex);
		if (controlID < ControlIdFirstHardware || hardwareParams.count(controlID) != 1)
		{
			return false;
		}

		if (hardwareParams.count(controlID) != 1)
		{
			return false;
		}
		params = hardwareParams.at(controlID);
		if (params == nullptr)
		{
			return false;
		}
	}
	{
		std::lock_guard<std::mutex> Guard(controlMutex);
		if (controls.count(controlID) != 1)
		{
			return false;
		}
		CommandInterface* control = controls.at(controlID);
		controls.erase(controlID);
		delete control;
	}

	hardwareParams.erase(controlID);
	delete params;
	if (storage)
	{
		storage->deleteHardwareParams(controlID);
	}
	return true;
}

HardwareParams* Manager::getHardware(controlID_t controlID)
{
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) != 1)
	{
		return NULL;
	}
	return hardwareParams.at(controlID);
}

unsigned int Manager::controlsOfHardwareType(const hardwareType_t hardwareType)
{
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	unsigned int counter = 0;
	for (auto hardwareParam : hardwareParams)
	{
		if (hardwareParam.second->hardwareType == hardwareType)
		{
			counter++;
		}
	}
	return counter;
}

bool Manager::hardwareLibraryAdd(const hardwareType_t hardwareType, void* libraryHandle)
{
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) == 1)
	{
		return false;
	}
	hardwareLibraries[hardwareType] = libraryHandle;
	return true;
}

void* Manager::hardwareLibraryGet(const hardwareType_t hardwareType) const
{
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) != 1)
	{
		return nullptr;
	}
	return hardwareLibraries.at(hardwareType);
}

bool Manager::hardwareLibraryRemove(const hardwareType_t hardwareType)
{
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) != 1)
	{
		return false;
	}
	hardwareLibraries.erase(hardwareType);
	return true;
}

const std::string Manager::getControlName(const controlID_t controlID)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	if (controls.count(controlID) != 1)
	{
		return unknownControl;
	}
	CommandInterface* c = controls.at(controlID);
	return c->getName();
}

const std::map<controlID_t,std::string> Manager::controlListNames() const
{
	std::map<controlID_t,std::string> ret;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	for (auto hardware : hardwareParams)
	{
		ret[hardware.first] = hardware.second->name;
	}
	return ret;
}

const std::map<protocol_t,std::string> Manager::protocolsOfControl(const controlID_t controlID) const
{
	std::map<protocol_t,std::string> ret;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) != 1)
	{
		ret[ProtocolNone] = protocolSymbols[ProtocolNone];
	}

	std::lock_guard<std::mutex> Guard2(controlMutex);
	for (auto control : controls)
	{
		if (control.second->getManagerID() != ControlTypeHardware)
		{
			continue;
		}

		const HardwareHandler* hardware = static_cast<const HardwareHandler*>(control.second);
		if (hardware->getControlID() != controlID)
		{
			continue;
		}

		// FIXME: is this a memory leak, when I twice add the same protocol?
		std::vector<protocol_t> protocols;
		hardware->getProtocols(protocols);
		for (auto protocol : protocols)
		{
			ret[protocol] = protocolSymbols[protocol];
		}
	}
	return ret;
}

/***************************
* Loco                     *
***************************/

datamodel::Loco* Manager::getLoco(const locoID_t locoID) const
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) != 1)
	{
		return NULL;
	}
	return locos.at(locoID);
}

const std::string& Manager::getLocoName(const locoID_t locoID)
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) != 1)
	{
		return unknownLoco;
	}
	return locos.at(locoID)->name;
}

bool Manager::locoSave(const locoID_t locoID, const string& name, const controlID_t controlID, const protocol_t protocol, const address_t address, const function_t nr, string& result)
{
	Loco* loco;
	if (!checkControlProtocolAddress(AddressTypeLoco, controlID, protocol, address, result))
	{
		return false;
	}
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		if (locoID != LocoNone && locos.count(locoID))
		{
			// update existing loco
			loco = locos.at(locoID);
			if (loco == nullptr)
			{
				result.assign("Loco does not exist");
				return false;
			}
			loco->name = name;
			loco->controlID = controlID;
			loco->protocol = protocol;
			loco->address = address;
			loco->SetNrOfFunctions(nr);
		}
		else
		{
			// create new loco
			locoID_t newLocoID = 0;
			// get next locoID
			for (auto loco : locos)
			{
				if (loco.first > newLocoID)
				{
					newLocoID = loco.first;
				}
			}
			++newLocoID;
			loco = new Loco(this, newLocoID, name, controlID, protocol, address, nr);
			if (loco == nullptr)
			{
				result.assign("Unable to allocate memory for loco");
				return false;
			}
			// save in map
			locos[newLocoID] = loco;
		}
	}
	// save in db
	if (storage)
	{
		storage->loco(*loco);
	}
	return true;
}

bool Manager::locoDelete(const locoID_t locoID)
{
	Loco* loco = nullptr;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		if (locoID == LocoNone || locos.count(locoID) == 0)
		{
			return false;
		}

		loco = locos.at(locoID);
		if (loco->isInUse())
		{
			return false;
		}

		locos.erase(locoID);
	}

	delete loco;
	if (storage)
	{
		storage->deleteLoco(locoID);
	}
	return true;
}

bool Manager::locoProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) < 1)
	{
		controlID = 0;
		protocol = ProtocolNone;
		address = 0;
		return false;
	}
	Loco* loco = locos.at(locoID);
	controlID = loco->controlID;
	protocol = loco->protocol;
	address = loco->address;
	return true;
}

void Manager::locoSpeed(const controlType_t managerID, const protocol_t protocol, const address_t address, const LocoSpeed speed)
{
	locoID_t locoID = LocoNone;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		for (auto loco : locos) {
			if (loco.second->protocol == protocol && loco.second->address == address)
			{
				locoID = loco.first;
				break;
			}
		}
	}
	if (locoID == LocoNone)
	{
		return;
	}
	locoSpeed(managerID, locoID, speed);
}

bool Manager::locoSpeed(const controlType_t managerID, const locoID_t locoID, const LocoSpeed speed)
{
	Loco* loco = getLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	LocoSpeed s = speed;
	if (speed > MaxSpeed)
	{
		s = MaxSpeed;
	}
	xlog("%s (%i) speed is now %i", loco->name.c_str(), locoID, s);
	loco->Speed(s);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoSpeed(managerID, locoID, s);
	}
	return true;
}

const LocoSpeed Manager::locoSpeed(const locoID_t locoID) const
{
	Loco* loco = getLoco(locoID);
	if (loco == nullptr)
	{
		return MinSpeed;
	}
	return loco->Speed();
}

void Manager::locoDirection(const controlType_t managerID, const protocol_t protocol, const address_t address, const direction_t direction)
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	for (auto loco : locos)
	{
		if (loco.second->protocol == protocol && loco.second->address == address)
		{
			locoDirection(managerID, loco.first, direction);
			return;
		}
	}
}
void Manager::locoDirection(const controlType_t managerID, const locoID_t locoID, const direction_t direction)
{
	Loco* loco = getLoco(locoID);
	loco->SetDirection(direction);
	xlog("%s (%i) direction is now %i", loco->name.c_str(), locoID, direction);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoDirection(managerID, locoID, direction);
	}
}

void Manager::locoFunction(const controlType_t managerID, const locoID_t locoID, const function_t function, const bool on)
{
	Loco* loco = getLoco(locoID);
	loco->SetFunction(function, on);
	xlog("%s (%i) function %i is now %s", loco->name.c_str(), locoID, function, (on ? "on" : "off"));
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoFunction(managerID, locoID, function, on);
	}
}

/***************************
* Accessory                *
***************************/

void Manager::accessory(const controlType_t managerID, const protocol_t protocol, const address_t address, const accessoryState_t state)
{
	accessoryID_t accessoryID = AccessoryNone;
	{
		std::lock_guard<std::mutex> Guard(accessoryMutex);
		for (auto accessory : accessories)
		{
			if (accessory.second->protocol == protocol && accessory.second->address == address)
			{
				accessoryID = accessory.first;
				break;
			}
		}
	}
	if (accessoryID == AccessoryNone)
	{
		return;
	}
	accessory(managerID, accessoryID, state);
}

void Manager::accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state)
{
	Accessory* accessory = getAccessory(accessoryID);
	if (accessory == nullptr)
	{
		return;
	}
	accessory->state = state;

	this->accessory(managerID, accessoryID, state, accessory->IsInverted(), true);

	delayedCall->Accessory(managerID, accessoryID, state, accessory->IsInverted(), accessory->timeout);
}

void Manager::accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const bool on)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		accessoryState_t tempState = (control.first >= ControlIdFirstHardware ? (state != inverted) : state);
		control.second->accessory(managerID, accessoryID, tempState, on);
	}
}

Accessory* Manager::getAccessory(const accessoryID_t accessoryID)
{
	std::lock_guard<std::mutex> Guard(accessoryMutex);
	if (accessories.count(accessoryID) != 1)
	{
		return nullptr;
	}
	return accessories.at(accessoryID);
}

const std::string& Manager::getAccessoryName(const accessoryID_t accessoryID)
{
	if (accessories.count(accessoryID) != 1)
	{
		return unknownAccessory;
	}
	return accessories.at(accessoryID)->name;
}

bool Manager::checkAccessoryPosition(const accessoryID_t accessoryID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ)
{
	Accessory* accessory = getAccessory(accessoryID);
	if (accessory == nullptr)
	{
		return false;
	}

	return (accessory->posX == posX && accessory->posY == posY && accessory->posZ == posZ);
}
bool Manager::accessorySave(const accessoryID_t accessoryID, const string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryType_t type, const accessoryTimeout_t timeout, const bool inverted, string& result)
{
	Accessory* accessory;
	if (!checkControlProtocolAddress(AddressTypeAccessory, controlID, protocol, address, result))
	{
		result.append("Invalid control-protocol-address combination.");
		return false;
	}

	if (!checkAccessoryPosition(accessoryID, posX, posY, posZ) && !checkPositionFree(posX, posY, posZ, Width1, Height1, Rotation0, result))
	{
		result.append("Unable to ");
		result.append(accessoryID == AccessoryNone ? "add" : "move");
		result.append(" accessory.");
		return false;
	}

	{
		std::lock_guard<std::mutex> Guard(accessoryMutex);
		if (accessoryID != AccessoryNone && accessories.count(accessoryID))
		{
			// update existing accessory
			accessory = accessories.at(accessoryID);
			if (accessory == nullptr)
			{
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
			accessory->timeout = timeout;
			accessory->Inverted(inverted);
		}
		else
		{
			// create new accessory
			accessoryID_t newAccessoryID = 0;
			// get next accessoryID
			for (auto accessory : accessories)
			{
				if (accessory.first > newAccessoryID)
				{
					newAccessoryID = accessory.first;
				}
			}
			++newAccessoryID;
			accessory = new Accessory(newAccessoryID, name, posX, posY, posZ, Rotation0, controlID, protocol, address, type, timeout, inverted);
			if (accessory == nullptr)
			{
				result.assign("Unable to allocate memory for accessory");
				return false;
			}
			// save in map
			accessories[newAccessoryID] = accessory;
		}
	}
	// save in db
	if (storage)
	{
		storage->accessory(*accessory);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->accessorySettings(accessory->objectID, name, posX, posY, posZ);
	}
	return true;
}

bool Manager::accessoryDelete(const accessoryID_t accessoryID)
{
	Accessory* accessory = nullptr;
	{
		std::lock_guard<std::mutex> Guard(accessoryMutex);
		if (accessoryID == AccessoryNone || accessories.count(accessoryID) == 0)
		{
			return false;
		}

		accessory = accessories.at(accessoryID);
		accessories.erase(accessoryID);
	}

	string name = accessory->name;

	delete accessory;
	if (storage)
	{
		storage->deleteAccessory(accessoryID);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->accessoryDelete(accessoryID, name);
	}
	return true;
}

bool Manager::accessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const
{
	if (accessories.count(accessoryID) < 1)
	{
		controlID = 0;
		protocol = ProtocolNone;
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

void Manager::feedback(const controlType_t managerID, const feedbackPin_t pin, const feedbackState_t state)
{
	Feedback* feedback = getFeedback(pin);
	if (feedback == nullptr)
	{
		return;
	}
	xlog("Feedback %i is now %s", pin, (state ? "on" : "off"));
	feedback->setState(state);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->feedback(managerID, pin, state);
	}
}

datamodel::Feedback* Manager::getFeedback(feedbackID_t feedbackID)
{
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	if (feedbacks.count(feedbackID) != 1)
	{
		return NULL;
	}
	return feedbacks.at(feedbackID);
}

const std::string& Manager::getFeedbackName(const feedbackID_t feedbackID)
{
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	if (feedbacks.count(feedbackID) != 1)
	{
		return unknownFeedback;
	}
	return feedbacks.at(feedbackID)->name;
}

bool Manager::feedbackSave(const feedbackID_t feedbackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const feedbackPin_t pin, const bool inverted, string& result)
{
	Feedback* feedback;
	{
		if (!checkPositionFree(posX, posY, posZ, Width1, Height1, Rotation0, result))
		{
			result.append(" Unable to ");
			result.append(feedbackID == FeedbackNone ? "add" : "move");
			result.append(" feedback.");
			return false;
		}
		std::lock_guard<std::mutex> Guard(feedbackMutex);
		if (feedbackID != FeedbackNone && feedbacks.count(feedbackID))
		{
			// update existing feedback
			feedback = feedbacks.at(feedbackID);
			if (feedback == nullptr)
			{
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
		else
		{
			// create new feedback
			feedbackID_t newFeedbackID = 0;
			// get next feedbackID
			for (auto feedback : feedbacks) {
				if (feedback.first > newFeedbackID)
				{
					newFeedbackID = feedback.first;
				}
			}
			++newFeedbackID;
			feedback = new Feedback(this, newFeedbackID, name, posX, posY, posZ, controlID, pin, inverted);
			if (feedback == nullptr)
			{
				result.assign("Unable to allocate memory for feedback");
				return false;
			}
			// save in map
			feedbacks[newFeedbackID] = feedback;
		}
	}
	// save in db
	if (storage)
	{
		storage->feedback(*feedback);
	}
	return feedback;
}

bool Manager::feedbackDelete(const feedbackID_t feedbackID)
{
	Feedback* feedback = nullptr;
	{
		std::lock_guard<std::mutex> Guard(feedbackMutex);
		if (feedbackID == FeedbackNone || feedbacks.count(feedbackID) == 0)
		{
			return false;
		}

		feedback = feedbacks.at(feedbackID);
		if (feedback == nullptr)
		{
			return false;
		}

		feedbacks.erase(feedbackID);
	}

	delete feedback;
	if (storage)
	{
		storage->deleteFeedback(feedbackID);
	}
	return true;
}

/***************************
* Track                    *
***************************/

void Manager::track(const controlType_t managerID, const trackID_t trackID, const lockState_t state)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->track(managerID, trackID, state);
	}
}

Track* Manager::getTrack(const trackID_t trackID)
{
	std::lock_guard<std::mutex> Guard(trackMutex);
	if (tracks.count(trackID) != 1)
	{
		return NULL;
	}
	return tracks.at(trackID);
}

const std::string& Manager::getTrackName(const trackID_t trackID)
{
	if (tracks.count(trackID) != 1)
	{
		return unknownTrack;
	}
	return tracks.at(trackID)->name;
}

bool Manager::trackSave(const trackID_t trackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutRotation_t rotation, const trackType_t type, string& result)
{
	Track* track;
	{
		if (!checkPositionFree(posX, posY, posZ, width, Height1, rotation, result))
		{
			result.append(" Unable to ");
			result.append(trackID == TrackNone ? "add" : "move");
			result.append(" track.");
			return false;
		}
		std::lock_guard<std::mutex> Guard(trackMutex);
		if (trackID != TrackNone && tracks.count(trackID))
		{
			// update existing track
			track = tracks.at(trackID);
			if (track == nullptr)
			{
				result.assign("Track does not exist");
				return false;
			}
			track->name = name;
			track->width = width;
			track->rotation = rotation;
			track->posX = posX;
			track->posY = posY;
			track->posZ = posZ;
			track->Type(type);
		}
		else
		{
			// create new track
			trackID_t newTrackID = 0;
			// get next trackID
			for (auto track : tracks)
			{
				if (track.first > newTrackID)
				{
					newTrackID = track.first;
				}
			}
			++newTrackID;
			track = new Track(newTrackID, name, posX, posY, posZ, width, rotation, type);
			if (track == nullptr)
			{
				result.assign("Unable to allocate memory for track");
				return false;
			}
			// save in map
			tracks[newTrackID] = track;
		}
	}
	// save in db
	if (storage)
	{
		storage->track(*track);
	}
	return true;
}

bool Manager::trackDelete(const trackID_t trackID)
{
	Track* track = nullptr;
	{
		std::lock_guard<std::mutex> Guard(trackMutex);
		if (trackID == TrackNone || tracks.count(trackID) == 0)
		{
			return false;
		}

		track = tracks.at(trackID);
		if (track == nullptr || track->isInUse())
		{
			return false;
		}

		tracks.erase(trackID);
	}

	delete track;
	if (storage)
	{
		storage->deleteTrack(trackID);
	}
	return true;
}

/***************************
* Switch                   *
***************************/

void Manager::handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state)
{
	Switch* mySwitch = getSwitch(switchID);
	if (mySwitch == nullptr)
	{
		return;
	}
	mySwitch->state = state;

	this->handleSwitch(managerID, switchID, state, mySwitch->IsInverted(), true);

	delayedCall->Switch(managerID, switchID, state, mySwitch->IsInverted(), mySwitch->timeout);
}

void Manager::handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state, const bool inverted, const bool on)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		switchState_t tempState = (control.first >= ControlIdFirstHardware ? (state != inverted) : state);
		control.second->handleSwitch(managerID, switchID, tempState, on);
	}
}

Switch* Manager::getSwitch(const switchID_t switchID)
{
	std::lock_guard<std::mutex> Guard(switchMutex);
	if (switches.count(switchID) != 1)
	{
		return NULL;
	}
	return switches.at(switchID);
}

const std::string& Manager::getSwitchName(const switchID_t switchID)
{
	if (switches.count(switchID) != 1)
	{
		return unknownSwitch;
	}
	return switches.at(switchID)->name;
}

bool Manager::checkSwitchPosition(const switchID_t switchID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ)
{
	Switch* mySwitch = getSwitch(switchID);
	if (mySwitch == nullptr)
	{
		return false;
	}

	return (mySwitch->posX == posX && mySwitch->posY == posY && mySwitch->posZ == posZ);
}

bool Manager::switchSave(const switchID_t switchID, const string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutRotation_t rotation, const controlID_t controlID, const protocol_t protocol, const address_t address, const switchType_t type, const switchTimeout_t timeout, const bool inverted, string& result)
{
	Switch* mySwitch;
	if (!checkControlProtocolAddress(AddressTypeAccessory, controlID, protocol, address, result))
	{
		return false;
	}

	if (!checkSwitchPosition(switchID, posX, posY, posZ) && !checkPositionFree(posX, posY, posZ, Width1, Height1, rotation, result))
	{
		result.append("Unable to ");
		result.append(switchID == SwitchNone ? "add" : "move");
		result.append(" switch.");
		return false;
	}

	{
		std::lock_guard<std::mutex> Guard(switchMutex);
		if (switchID != SwitchNone && switches.count(switchID))
		{
			// update existing switch
			mySwitch = switches.at(switchID);
			if (mySwitch == nullptr)
			{
				result.assign("Switch does not exist");
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
			mySwitch->timeout = timeout;
			mySwitch->Inverted(inverted);
		}
		else
		{
			// create new switch
			switchID_t newSwitchID = 0;
			// get next switchID
			for (auto mySwitch : switches)
			{
				if (mySwitch.first > newSwitchID)
				{
					newSwitchID = mySwitch.first;
				}
			}
			++newSwitchID;
			mySwitch = new Switch(newSwitchID, name, posX, posY, posZ, rotation, controlID, protocol, address, type, timeout, inverted);
			if (mySwitch == nullptr)
			{
				result.assign("Unable to allocate memory for switch");
				return false;
			}
			// save in map
			switches[newSwitchID] = mySwitch;
		}
	}
	// save in db
	if (storage)
	{
		storage->saveSwitch(*mySwitch);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->switchSettings(mySwitch->objectID, name, posX, posY, posZ, mySwitch->Rotation());
	}
	return true;
}

bool Manager::switchDelete(const switchID_t switchID)
{
	Switch* mySwitch = nullptr;
	{
		std::lock_guard<std::mutex> Guard(switchMutex);
		if (switchID == SwitchNone || switches.count(switchID) == 0)
		{
			return false;
		}

		mySwitch = switches.at(switchID);
		switches.erase(switchID);
	}

	string name = mySwitch->name;

	delete mySwitch;
	if (storage)
	{
		storage->deleteSwitch(switchID);
	}

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->switchDelete(switchID, name);
	}
	return true;
}

bool Manager::switchProtocolAddress(const switchID_t switchID, controlID_t& controlID, protocol_t& protocol, address_t& address) const
{
	if (switches.count(switchID) < 1)
	{
		controlID = 0;
		protocol = ProtocolNone;
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

Street* Manager::getStreet(const streetID_t streetID)
{
	std::lock_guard<std::mutex> Guard(streetMutex);
	if (streets.count(streetID) != 1)
	{
		return NULL;
	}
	return streets.at(streetID);
}

const string& Manager::getStreetName(const streetID_t streetID)
{
	if (streets.count(streetID) != 1)
	{
		return unknownStreet;
	}
	return streets.at(streetID)->name;
}

bool Manager::streetSave(const streetID_t streetID, const std::string& name, const trackID_t fromTrack, const direction_t fromDirection, const trackID_t toTrack, const direction_t toDirection, const feedbackID_t feedbackID, string& result)
{
	Street* street;
	{
		std::lock_guard<std::mutex> Guard(streetMutex);
		if (streetID != StreetNone && streets.count(streetID))
		{
			// update existing street
			street = streets.at(streetID);
			if (street == nullptr)
			{
				result.assign("Street does not exist");
				return false;
			}
			street->name = name;
			street->fromTrack = fromTrack;
			street->fromDirection = fromDirection;
			street->toTrack = toTrack;
			street->toDirection = toDirection;
			street->feedbackIDStop = feedbackID;
		}
		else
		{
			// create new street
			streetID_t newStreetID = 0;
			// get next streetID
			for (auto street : streets)
			{
				if (street.first > newStreetID)
				{
					newStreetID = street.first;
				}
			}
			++newStreetID;
			street = new Street(this, newStreetID, name, fromTrack, fromDirection, toTrack, toDirection, feedbackID);
			if (street == nullptr)
			{
				result.assign("Unable to allocate memory for street");
				return false;
			}
			// save in map
			streets[newStreetID] = street;
		}
	}
	// save in db
	if (storage)
	{
		storage->street(*street);
	}
	return true;
}

bool Manager::streetDelete(const streetID_t streetID)
{
	Street* street = nullptr;
	{
		std::lock_guard<std::mutex> Guard(streetMutex);
		if (streetID == StreetNone || streets.count(streetID) == 0)
		{
			return false;
		}

		street = streets.at(streetID);
		streets.erase(streetID);
	}

	delete street;
	if (storage)
	{
		storage->deleteStreet(streetID);
	}
	return true;
}

/***************************
* Automode                 *
***************************/

bool Manager::locoIntoTrack(const locoID_t locoID, const trackID_t trackID)
{
	Track* track = getTrack(trackID);
	if (track == nullptr)
	{
		return false;
	}

	Loco* loco = getLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}

	bool reserved = track->reserve(locoID);
	if (reserved == false)
	{
		return false;
	}

	reserved = loco->toTrack(trackID);
	if (reserved == false)
	{
		track->release(locoID);
		return false;
	}

	reserved = track->lock(locoID);
	if (reserved == false)
	{
		loco->release();
		track->release(locoID);
		return false;
	}

	xlog("%s (%i) is now on track %s (%i)", loco->name.c_str(), loco->objectID, track->name.c_str(), track->objectID);

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoIntoTrack(locoID, trackID);
	}
	return true;
}

bool Manager::locoRelease(const locoID_t locoID)
{
	locoSpeed(ControlTypeAutomode, locoID, MinSpeed);

	Loco* loco = getLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	streetRelease(loco->street());
	trackRelease(loco->track());
	return loco->release();
}

bool Manager::trackRelease(const trackID_t trackID)
{
	Track* track = getTrack(trackID);
	if (track == nullptr)
	{
		return false;
	}
	locoID_t locoID = track->getLoco();
	return track->release(locoID);
}
bool Manager::feedbackRelease(const feedbackID_t feedbackID)
{
	Feedback* feedback = getFeedback(feedbackID);
	if (feedback == nullptr)
	{
		return false;
	}
	locoID_t locoID = feedback->getLoco();
	return feedback->release(locoID);
}

bool Manager::streetRelease(const streetID_t streetID)
{
	Street* street = getStreet(streetID);
	if (street == nullptr)
	{
		return false;
	}
	locoID_t locoID = street->getLoco();
	return street->release(locoID);
}

/*
bool Manager::switchRelease(const switchID_t switchID) {
	Switch* mySwitch = getSwitch(switchID);
	if (mySwitch == nullptr) {
		return false;
	}
	locoID_t locoID = mySwitch->getLoco();
	return mySwitch->release(locoID);
}
*/

bool Manager::locoStreet(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoStreet(locoID, streetID, trackID);
	}
	return true;
}
bool Manager::locoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoDestinationReached(locoID, streetID, trackID);
	}
	return true;
}

bool Manager::locoStart(const locoID_t locoID)
{
	Loco* loco = getLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	bool ret = loco->start();
	if (ret == false)
	{
		return false;
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoStart(locoID);
	}
	return true;
}

bool Manager::locoStop(const locoID_t locoID)
{
	Loco* loco = getLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	bool ret = loco->stop();
	if (ret == false)
	{
		return false;
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoStop(locoID);
	}
	return true;
}

bool Manager::locoStartAll()
{
	for (auto loco : locos)
	{
		bool ret = loco.second->start();
		if (ret == false)
		{
			continue;
		}
		{
			std::lock_guard<std::mutex> Guard(controlMutex);
			for (auto control : controls)
			{
				control.second->locoStart(loco.first);
			}
		}
	}
	return true;
}

bool Manager::locoStopAll()
{
	bool ret1 = true;
	for (auto loco : locos)
	{
		if (!loco.second->isInUse())
		{
			continue;
		}
		bool ret2 = loco.second->stop();
		ret1 &= ret2;
		if (ret2 == false)
		{
			continue;
		}
		{
			std::lock_guard<std::mutex> Guard(controlMutex);
			for (auto control : controls)
			{
				control.second->locoStop(loco.first);
			}
		}
	}
	return ret1;
}

/***************************
* Default Values           *
***************************/

void Manager::loadDefaultValuesToDB()
{
	HardwareParams newHardwareParams1(1, HardwareTypeVirtual, "Virtuelle Zentrale", "");
	storage->hardwareParams(newHardwareParams1);

	HardwareParams newHardwareParams2(2, HardwareTypeCS2, "CS2 Zentrale", "192.168.0.190");
	storage->hardwareParams(newHardwareParams2);

	Loco newloco1(this, 1, "Re 460 Teddy", 1, ProtocolDCC, 1119, 4);
	storage->loco(newloco1);

	Loco newloco2(this, 2, "ICN", 1, ProtocolDCC, 1118, 4);
	storage->loco(newloco2);

	Accessory newAccessory1(1, "Schalter 1", 3, 5, 0, Rotation0, 1, ProtocolDCC, 1, 1, 200, false);
	storage->accessory(newAccessory1);

	Accessory newAccessory2(2, "Schalter 2", 3, 6, 0, Rotation0, 1, ProtocolDCC, 2, 1, 200, false);
	storage->accessory(newAccessory2);

	Feedback newFeedback1(this, 1, "Rückmelder Bahnhof 1", 1, 1, 4, 5, 0, false);
	storage->feedback(newFeedback1);

	Feedback newFeedback2(this, 2, "Rückmelder Bahnhof 2", 1, 2, 4, 6, 0, false);
	storage->feedback(newFeedback2);

	Feedback newFeedback3(this, 3, "Rückmelder Ausfahrt", 1, 3, 4, 6, 0, false);
	storage->feedback(newFeedback3);

	Feedback newFeedback4(this, 4, "Rückmelder Strecke", 1, 4, 4, 6, 0, false);
	storage->feedback(newFeedback4);

	Feedback newFeedback5(this, 5, "Rückmelder Einfahrt", 1, 5, 4, 6, 0, false);
	storage->feedback(newFeedback5);

	Track newTrack1(1, "Gleis Bahnhof 1", 4, 5, 5, 0, Rotation0, TrackTypeStraight);
	storage->track(newTrack1);

	Track newTrack2(2, "Gleis Bahnhof 2", 4, 5, 6, 0, Rotation90, TrackTypeStraight);
	storage->track(newTrack2);

	Track newTrack3(3, "Gleis Ausfahrt", 4, 5, 6, 0, Rotation90, TrackTypeStraight);
	storage->track(newTrack3);

	Track newTrack4(4, "Gleis Einfahrt", 4, 5, 6, 0, Rotation90, TrackTypeStraight);
	storage->track(newTrack4);

	Track newTrack5(5, "Gleis Strecke", 4, 5, 6, 0, Rotation90, TrackTypeStraight);
	storage->track(newTrack5);

	Switch newSwitch1(1, "Weiche Einfahrt", 2, 5, 0, Rotation90, 1, ProtocolDCC, 3, SwitchTypeLeft, 200, false);
	storage->saveSwitch(newSwitch1);

	Switch newSwitch2(2, "Weiche Ausfahrt", 2, 6, 0, Rotation0, 1, ProtocolDCC, 4, SwitchTypeRight, 200, false);
	storage->saveSwitch(newSwitch2);

	Street newStreet1(this, 1, "Fahrstrasse Ausfahrt 1", 1, DirectionLeft, 3, DirectionLeft, 3);
	storage->street(newStreet1);

	Street newStreet2(this, 2, "Fahrstrasse Ausfahrt 2", 2, DirectionLeft, 3, DirectionLeft, 3);
	storage->street(newStreet2);

	Street newStreet3(this, 3, "Fahrstrasse Auf Strecke", 3, DirectionLeft, 4, DirectionLeft, 4);
	storage->street(newStreet3);

	Street newStreet4(this, 4, "Fahrstrasse Von Strecke", 4, DirectionLeft, 5, DirectionLeft, 5);
	storage->street(newStreet4);

	Street newStreet5(this, 5, "Fahrstrasse Einfahrt 1", 5, DirectionLeft, 1, DirectionLeft, 1);
	storage->street(newStreet5);

	Street newStreet6(this, 6, "Fahrstrasse Einfahrt 2", 5, DirectionLeft, 2, DirectionLeft, 2);
	storage->street(newStreet6);
}

/***************************
* Layout                   *
***************************/

bool Manager::checkPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation, string& result)
{
	if (width == 0 || height == 0)
	{
		result.assign("Width or height is zero.");
		return false;
	}
	layoutPosition_t x;
	layoutPosition_t y;
	layoutPosition_t z = posZ;
	layoutItemSize_t w;
	layoutItemSize_t h;
	bool ret = datamodel::LayoutItem::mapPosition(posX, posY, width, height, rotation, x, y, w, h);
	if (ret == false)
	{
		return false;
	}
	for(layoutPosition_t ix = x; ix < x + w; ix++)
	{
		for(layoutPosition_t iy = y; iy < y + h; iy++)
		{
			bool ret = checkLayoutPositionFree(ix, iy, z, result, accessories, accessoryMutex);
			if (ret == false)
			{
				return false;
			}
			ret = checkLayoutPositionFree(ix, iy, z, result, tracks, trackMutex);
			if (ret == false)
			{
				return false;
			}
			ret = checkLayoutPositionFree(ix, iy, z, result, feedbacks, feedbackMutex);
			if (ret == false)
			{
				return false;
			}
			ret = checkLayoutPositionFree(ix, iy, z, result, switches, switchMutex);
			if (ret == false)
			{
				return false;
			}
		}
	}
	return true;
}

template<class Type>
bool Manager::checkLayoutPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, string& result, map<objectID_t, Type*>& layoutVector, std::mutex& mutex)
{
	std::lock_guard<std::mutex> Guard(mutex);
	for (auto layout : layoutVector)
	{
		if (layout.second->checkPositionFree(posX, posY, posZ))
		{
			continue;
		}
		stringstream status;
		status << "Position " << static_cast<int>(posX) << "/" << static_cast<int>(posY) << "/" << static_cast<int>(posZ) << " is already used by " << layout.second->layoutType() << " \"" << layout.second->name << "\".";
		result.assign(status.str());
		return false;
	}
	return true;
}

bool Manager::checkAddressLoco(const protocol_t protocol, const address_t address, string& result)
{
	switch (protocol)
	{
		case ProtocolDCC:
			if (address > 10239)
			{
				result.assign("Address higher then 10239 is not supported by DCC");
				return false;
			}
			return true;

		case ProtocolMM1:
		case ProtocolMM2:
			if (address > 80)
			{
				result.assign("Address higher then 80 is not supported by MM1/MM2");
				return false;
			}
			return true;

		default:
			return true;
	}
}

bool Manager::checkAddressAccessory(const protocol_t protocol, const address_t address, string& result)
{
	switch (protocol)
	{
		case ProtocolDCC:
			if (address > 2044)
			{
				result.assign("Address higher then 2044 is not supported by DCC");
				return false;
			}
			return true;

		case ProtocolMM1:
		case ProtocolMM2:
			if (address > 320) {
				result.assign("Address higher then 320 is not supported by MM1/MM2");
				return false;
			}
			return true;

		default:
			return true;
	}
}

bool Manager::checkControlProtocolAddress(const addressType_t type, const controlID_t controlID, const protocol_t protocol, const address_t address, string& result)
{
	{
		std::lock_guard<std::mutex> Guard(controlMutex);
		if (controlID < ControlIdFirstHardware || controls.count(controlID) != 1)
		{
			result.assign("Control does not exist");
			return false;
		}
		CommandInterface* control = controls.at(controlID);
		if (!control->protocolSupported(protocol))
		{
			stringstream status;
			status << "Protocol " << static_cast<int>(protocol);
			if (protocol > ProtocolNone && protocol <= ProtocolEnd)
			{
				status << "/" << protocolSymbols[protocol];
			}
			status << " is not supported by control. Please use one of: ";
			std::vector<protocol_t> protocols;
			control->getProtocols(protocols);
			for (auto p : protocols)
			{
				status << static_cast<int>(p) << "/" << protocolSymbols[p] << " ";
			}
			result.assign(status.str());
			return false;
		}
	}
	if (address == 0)
	{
		result.assign("Address must be higher then 0");
		return false;
	}
	switch (type)
	{
		case AddressTypeLoco:
			return checkAddressLoco(protocol, address, result);

		case AddressTypeAccessory:
			return checkAddressAccessory(protocol, address, result);

		default:
			return false;
	}
}

