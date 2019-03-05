#include <future>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "console/ConsoleServer.h"
#include "datamodel/layout_item.h"
#include "DelayedCall.h"
#include "hardware/HardwareHandler.h"
#include "hardware/HardwareParams.h"
#include "manager.h"
#include "railcontrol.h"
#include "util.h"
#include "webserver/webserver.h"

using console::ConsoleServer;
using datamodel::Accessory;
using datamodel::Track;
using datamodel::Feedback;
using datamodel::Layer;
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
:	logger(Logger::Logger::GetLogger("Manager")),
	storage(nullptr),
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
		logger->Info("Unable to create storage handler");
		return;
	}

	//loadDefaultValuesToDB();

	controls[ControlIdConsole] = new ConsoleServer(*this, config.getValue("consoleport", 2222));
	controls[ControlIdWebserver] = new WebServer(*this, config.getValue("webserverport", 80));

	storage->allHardwareParams(hardwareParams);
	for (auto hardwareParam : hardwareParams)
	{
		hardwareParam.second->manager = this;
		controls[hardwareParam.second->controlID] = new HardwareHandler(*this, hardwareParam.second);
		logger->Info("Loaded control {0}: {1}", hardwareParam.first, hardwareParam.second->name);
	}

	storage->allLayers(layers);
	for (auto layer : layers)
	{
		logger->Info("Loaded layer {0}: {1}", layer.second->objectID, layer.second->Name());
	}
	if (layers.count(1) == 0)
	{
		string result;
		bool initLayer0 = LayerSave(0, "Layer 0", result);
		if (initLayer0 == false)
		{
			logger->Error("Unable to add initial layer 0");
		}
	}

	storage->allLocos(locos);
	for (auto loco : locos)
	{
		logger->Info("Loaded loco {0}: {1}", loco.second->objectID, loco.second->name);
	}

	storage->allAccessories(accessories);
	for (auto accessory : accessories)
	{
		logger->Info("Loaded accessory {0}: {1}", accessory.second->objectID, accessory.second->name);
	}

	storage->allFeedbacks(feedbacks);
	for (auto feedback : feedbacks)
	{
		logger->Info("Loaded feedback {0}: {1}", feedback.second->objectID, feedback.second->name);
	}

	storage->allTracks(tracks);
	for (auto track : tracks)
	{
		logger->Info("Loaded track {0}: {1}", track.second->objectID, track.second->name);
	}

	storage->allSwitches(switches);
	for (auto mySwitch : switches)
	{
		logger->Info("Loaded switch {0}: {1}", mySwitch.second->objectID, mySwitch.second->name);
	}

	storage->allStreets(streets);
	for (auto street : streets)
	{
		logger->Info("Loaded street {0}: {1}", street.second->objectID, street.second->name);
	}
	// FIXME: load locos into tracks
}

Manager::~Manager()
{
	while (!locoStopAll())
	{
		sleep(1);
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
		logger->Info("Unloading control {0}: {1}", controlID, params->name);
		delete control.second;
		hardwareParams.erase(controlID);
		delete params;
	}

	if (storage == nullptr)
	{
		return;
	}

	for (auto street : streets)
	{
		logger->Info("Saving street {0}: {1}", street.second->objectID, street.second->name);
		storage->street(*(street.second));
		delete street.second;
	}

	for (auto mySwitch : switches)
	{
		logger->Info("Saving switch {0}: {1}", mySwitch.second->objectID, mySwitch.second->name);
		storage->saveSwitch(*(mySwitch.second));
		delete mySwitch.second;
	}

	for (auto accessory : accessories)
	{
		logger->Info("Saving accessory {0}: {1}", accessory.second->objectID, accessory.second->name);
		storage->accessory(*(accessory.second));
		delete accessory.second;
	}

	for (auto  feedback : feedbacks)
	{
		logger->Info("Saving feedback {0}: {1}", feedback.second->objectID, feedback.second->name);
		storage->feedback(*(feedback.second));
		delete feedback.second;
	}

	for (auto track : tracks)
	{
		logger->Info("Saving track {0}: {1}", track.second->objectID, track.second->name);
		storage->track(*(track.second));
		delete track.second;
	}

	for (auto loco : locos)
	{
		logger->Info("Saving loco {0}: {1}", loco.second->objectID, loco.second->name);
		storage->loco(*(loco.second));
		delete loco.second;
	}

	for (auto layer : layers)
	{
		logger->Info("Saving layer {0}: {1}", layer.second->objectID, layer.second->Name());
		storage->layer(*(layer.second));
		delete layer.second;
	}

	delete delayedCall;
	delayedCall = nullptr;

	delete storage;
	storage = nullptr;
}

/***************************
* Booster                  *
***************************/

void Manager::booster(const controlType_t controlType, const boosterStatus_t status)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->booster(controlType, status);
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
	hardwareList[HardwareTypeRM485] = "RM485";
	hardwareList[HardwareTypeVirtual] = "Virtual Command Station (no Hardware)";
	return hardwareList;
}

bool Manager::controlSave(const controlID_t& controlID,
	const hardwareType_t& hardwareType,
	const std::string& name,
	const std::string& arg1,
	const std::string& arg2,
	const std::string& arg3,
	const std::string& arg4,
	const std::string& arg5,
	string& result)
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
			params->arg2 = arg2;
			params->arg3 = arg3;
			params->arg4 = arg4;
			params->arg5 = arg5;
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
			params = new HardwareParams(newControlID, hardwareType, name, arg1, arg2, arg3, arg4, arg5);
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

const std::map<controlID_t,std::string> Manager::LocoControlListNames() const
{
	std::map<controlID_t,std::string> ret;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	for (auto hardware : hardwareParams)
	{
		std::lock_guard<std::mutex> Guard2(controlMutex);
		if (controls.count(hardware.second->controlID) != 1)
		{
			continue;
		}
		CommandInterface* c = controls.at(hardware.second->controlID);
		if (c->CanHandleLocos() == false)
		{
			continue;
		}
		ret[hardware.first] = hardware.second->name;
	}
	return ret;
}

const std::map<controlID_t,std::string> Manager::AccessoryControlListNames() const
{
	std::map<controlID_t,std::string> ret;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	for (auto hardware : hardwareParams)
	{
		std::lock_guard<std::mutex> Guard2(controlMutex);
		if (controls.count(hardware.second->controlID) != 1)
		{
			continue;
		}
		CommandInterface* c = controls.at(hardware.second->controlID);
		if (c->CanHandleAccessories() == false)
		{
			continue;
		}
		ret[hardware.first] = hardware.second->name;
	}
	return ret;
}

const std::map<controlID_t,std::string> Manager::FeedbackControlListNames() const
{
	std::map<controlID_t,std::string> ret;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	for (auto hardware : hardwareParams)
	{
		std::lock_guard<std::mutex> Guard2(controlMutex);
		if (controls.count(hardware.second->controlID) != 1)
		{
			continue;
		}
		CommandInterface* c = controls.at(hardware.second->controlID);
		if (c->CanHandleFeedback() == false)
		{
			continue;
		}
		ret[hardware.first] = hardware.second->name;
	}
	return ret;
}

const map<string,hardware::HardwareParams*> Manager::controlListByName() const
{
	map<string,hardware::HardwareParams*> out;
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	for(auto hardware : hardwareParams)
	{
		out[hardware.second->name] = hardware.second;
	}
	return out;
}

const std::map<std::string, protocol_t> Manager::ProtocolsOfControl(const addressType_t type, const controlID_t controlID) const
{
	std::map<std::string,protocol_t> ret;
	{
		std::lock_guard<std::mutex> Guard(controlMutex);
		if (controls.count(controlID) != 1)
		{
			ret[protocolSymbols[ProtocolNone]] = ProtocolNone;
			return ret;
		}

		CommandInterface* control = controls.at(controlID);
		if (control->getcontrolType() != ControlTypeHardware)
		{
			ret[protocolSymbols[ProtocolNone]] = ProtocolNone;
			return ret;
		}

		const HardwareHandler* hardware = static_cast<const HardwareHandler*>(control);
		if (hardware->getControlID() != controlID)
		{
			ret[protocolSymbols[ProtocolNone]] = ProtocolNone;
			return ret;
		}

		std::vector<protocol_t> protocols;
		if (type == AddressTypeLoco)
		{
			hardware->GetLocoProtocols(protocols);
		}
		else
		{
			hardware->GetAccessoryProtocols(protocols);
		}
		for (auto protocol : protocols)
		{
			ret[protocolSymbols[protocol]] = protocol;
		}
	}
	return ret;
}

const std::map<unsigned char,argumentType_t> Manager::ArgumentTypesOfControl(const controlID_t controlID) const
{
	std::map<unsigned char,argumentType_t> ret;
	std::lock_guard<std::mutex> Guard(controlMutex);
	if (controls.count(controlID) != 1)
	{
		return ret;
	}

	const CommandInterface* control = controls.at(controlID);
	control->GetArgumentTypes(ret);
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

const map<string,datamodel::Loco*> Manager::locoListByName() const
{
	map<string,datamodel::Loco*> out;
	std::lock_guard<std::mutex> Guard(locoMutex);
	for(auto loco : locos)
	{
		out[loco.second->name] = loco.second;
	}
	return out;
}

bool Manager::locoSave(const locoID_t locoID, const string& name, const controlID_t controlID, const protocol_t protocol, const address_t address, const function_t nr, string& result)
{
	Loco* loco;
	if (!checkControlLocoProtocolAddress(controlID, protocol, address, result))
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
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoSettings(loco->objectID, name);
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

	if (storage)
	{
		storage->deleteLoco(locoID);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoDelete(loco->objectID, loco->Name());
	}
	delete loco;
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

void Manager::LocoSpeed(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const locoSpeed_t speed)
{
	locoID_t locoID = LocoNone;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		for (auto loco : locos) {
			if (loco.second->controlID == controlID
				&& loco.second->protocol == protocol
				&& loco.second->address == address)
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
	LocoSpeed(controlType, locoID, speed);
}

bool Manager::LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed)
{
	Loco* loco = getLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	locoSpeed_t s = speed;
	if (speed > MaxSpeed)
	{
		s = MaxSpeed;
	}
	logger->Info("{0} ({1}) speed is now {2}", loco->name, locoID, s);
	loco->Speed(s);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoSpeed(controlType, locoID, s);
	}
	return true;
}

const locoSpeed_t Manager::LocoSpeed(const locoID_t locoID) const
{
	Loco* loco = getLoco(locoID);
	if (loco == nullptr)
	{
		return MinSpeed;
	}
	return loco->Speed();
}

void Manager::LocoDirection(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const direction_t direction)
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	for (auto loco : locos)
	{
		if (loco.second->controlID == controlID
			&& loco.second->protocol == protocol
			&& loco.second->address == address)
		{
			LocoDirection(controlType, loco.first, direction);
			return;
		}
	}
}
void Manager::LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction)
{
	Loco* loco = getLoco(locoID);
	loco->SetDirection(direction);
	logger->Info("{0} ({1}) direction is now {2}", loco->name, locoID, direction);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoDirection(controlType, locoID, direction);
	}
}

void Manager::LocoFunction(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const function_t function, const bool on)
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	for (auto loco : locos)
	{
		if (loco.second->controlID == controlID
			&& loco.second->protocol == protocol
			&& loco.second->address == address)
		{
			LocoFunction(controlType, loco.first, function, on);
			return;
		}
	}
}

void Manager::LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on)
{
	Loco* loco = getLoco(locoID);
	loco->SetFunction(function, on);
	logger->Info("{0} ({1}) function {2} is now {3}", loco->name, locoID, function, (on ? "on" : "off"));
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoFunction(controlType, locoID, function, on);
	}
}

/***************************
* Accessory                *
***************************/

void Manager::AccessoryState(const controlType_t controlType, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryState_t state)
{
	accessoryID_t accessoryID = AccessoryNone;
	{
		std::lock_guard<std::mutex> Guard(accessoryMutex);
		for (auto accessory : accessories)
		{
			if (accessory.second->controlID == controlID
				&& accessory.second->protocol == protocol
				&& accessory.second->address == address)
			{
				accessoryID = accessory.first;
				break;
			}
		}
	}
	if (accessoryID != AccessoryNone)
	{
		AccessoryState(controlType, accessoryID, state);
		return;
	}

	switchID_t switchID = SwitchNone;
	{
		std::lock_guard<std::mutex> Guard(switchMutex);
		for (auto mySwitch : switches)
		{
			if (mySwitch.second->controlID == controlID
				&& mySwitch.second->protocol == protocol
				&& mySwitch.second->address == address)
			{
				switchID = mySwitch.first;
				break;
			}
		}
	}
	if (switchID != SwitchNone)
	{
		SwitchState(controlType, switchID, state);
		return;
	}

	// FIXME: add code for signals
}

void Manager::AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state)
{
	Accessory* accessory = getAccessory(accessoryID);
	if (accessory == nullptr)
	{
		return;
	}
	accessory->state = state;

	this->AccessoryState(controlType, accessoryID, state, accessory->IsInverted(), true);

	delayedCall->Accessory(controlType, accessoryID, state, accessory->IsInverted(), accessory->timeout);
}

void Manager::AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool inverted, const bool on)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		accessoryState_t tempState = (control.first >= ControlIdFirstHardware ? (state != inverted) : state);
		control.second->AccessoryState(controlType, accessoryID, tempState, on);
	}
}

Accessory* Manager::getAccessory(const accessoryID_t accessoryID) const
{
	std::lock_guard<std::mutex> Guard(accessoryMutex);
	if (accessories.count(accessoryID) != 1)
	{
		return nullptr;
	}
	return accessories.at(accessoryID);
}

const std::string& Manager::getAccessoryName(const accessoryID_t accessoryID) const
{
	if (accessories.count(accessoryID) != 1)
	{
		return unknownAccessory;
	}
	return accessories.at(accessoryID)->name;
}

bool Manager::CheckAccessoryPosition(const accessoryID_t accessoryID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const
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
	if (!checkControlAccessoryProtocolAddress(controlID, protocol, address, result))
	{
		result.append("Invalid control-protocol-address combination.");
		return false;
	}

	if (!CheckAccessoryPosition(accessoryID, posX, posY, posZ) && !CheckPositionFree(posX, posY, posZ, Width1, Height1, Rotation0, result))
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

const map<string,datamodel::Accessory*> Manager::accessoryListByName() const
{
	map<string,datamodel::Accessory*> out;
	std::lock_guard<std::mutex> Guard(accessoryMutex);
	for(auto accessory : accessories)
	{
		out[accessory.second->name] = accessory.second;
	}
	return out;
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

	if (storage)
	{
		storage->deleteAccessory(accessoryID);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->accessoryDelete(accessoryID, accessory->Name());
	}
	delete accessory;
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

void Manager::FeedbackState(const controlType_t controlType, const controlID_t controlID, const feedbackPin_t pin, const feedbackState_t state)
{
	feedbackID_t feedbackID = FeedbackNone;
	{
		std::lock_guard<std::mutex> Guard(feedbackMutex);
		for (auto feedback : feedbacks)
		{
			if (feedback.second->controlID == controlID && feedback.second->pin == pin)
			{
				feedbackID = feedback.first;
				break;
			}
		}
	}
	if (feedbackID == AccessoryNone)
	{
		return;
	}
	return FeedbackState(controlType, feedbackID, state);
}

void Manager::FeedbackState(const controlType_t controlType, const feedbackID_t feedbackID, const feedbackState_t state)
{
	Feedback* feedback = getFeedback(feedbackID);
	if (feedback == nullptr)
	{
		return;
	}
	logger->Info("Feedback {0} is now {1}", feedback->Name(), (state ? "on" : "off"));
	feedback->SetState(state);
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->FeedbackState(controlType, feedbackID, state);
	}
}

datamodel::Feedback* Manager::getFeedback(feedbackID_t feedbackID) const
{
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	if (feedbacks.count(feedbackID) != 1)
	{
		return nullptr;
	}
	return feedbacks.at(feedbackID);
}

const std::string& Manager::getFeedbackName(const feedbackID_t feedbackID) const
{
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	if (feedbacks.count(feedbackID) != 1)
	{
		return unknownFeedback;
	}
	return feedbacks.at(feedbackID)->Name();
}

bool Manager::CheckFeedbackPosition(const feedbackID_t feedbackID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const
{
	Feedback* feedback = getFeedback(feedbackID);
	if (feedback == nullptr)
	{
		return false;
	}

	if (feedback->visible == VisibleNo)
	{
		return false;
	}

	return (feedback->posX == posX && feedback->posY == posY && feedback->posZ == posZ);
}

bool Manager::feedbackSave(const feedbackID_t feedbackID, const std::string& name, const visible_t visible, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const feedbackPin_t pin, const bool inverted, string& result)
{
	if (visible && !CheckFeedbackPosition(feedbackID, posX, posY, posZ) && !CheckPositionFree(posX, posY, posZ, Width1, Height1, Rotation0, result))
	{
		result.append(" Unable to ");
		result.append(feedbackID == FeedbackNone ? "add" : "move");
		result.append(" feedback.");
		return false;
	}
	Feedback* feedback;
	{
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
			feedback->visible = visible;
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
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->FeedbackSettings(feedback->objectID, name);
	}
	return feedback;
}

const map<string,datamodel::Feedback*> Manager::FeedbackListByName() const
{
	map<string,datamodel::Feedback*> out;
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	for(auto feedback : feedbacks)
	{
		out[feedback.second->name] = feedback.second;
	}
	return out;
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

	if (storage)
	{
		storage->deleteFeedback(feedbackID);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->FeedbackDelete(feedback->objectID, feedback->Name());
	}
	delete feedback;
	return true;
}

/***************************
* Track                    *
***************************/

void Manager::track(const controlType_t controlType, const trackID_t trackID, const lockState_t state)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->track(controlType, trackID, state);
	}
}

Track* Manager::getTrack(const trackID_t trackID) const
{
	std::lock_guard<std::mutex> Guard(trackMutex);
	if (tracks.count(trackID) != 1)
	{
		return NULL;
	}
	return tracks.at(trackID);
}

const std::string& Manager::getTrackName(const trackID_t trackID) const
{
	if (tracks.count(trackID) != 1)
	{
		return unknownTrack;
	}
	return tracks.at(trackID)->name;
}

const map<string,datamodel::Track*> Manager::trackListByName() const
{
	map<string,datamodel::Track*> out;
	std::lock_guard<std::mutex> Guard(trackMutex);
	for(auto track : tracks)
	{
		out[track.second->name] = track.second;
	}
	return out;
}

const map<string,trackID_t> Manager::trackListIdByName() const
{
	map<string,trackID_t> out;
	std::lock_guard<std::mutex> Guard(trackMutex);
	for(auto track : tracks)
	{
		out[track.second->name] = track.second->objectID;
	}
	return out;
}

bool Manager::CheckTrackPosition(const trackID_t trackID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t height, const layoutRotation_t rotation, string& result) const
{
	layoutPosition_t x1;
	layoutPosition_t y1;
	layoutPosition_t z1 = posZ;
	layoutItemSize_t w1;
	layoutItemSize_t h1;
	bool ret = datamodel::LayoutItem::MapPosition(posX, posY, Width1, height, rotation, x1, y1, w1, h1);
	if (ret == false)
	{
		result = "Unable to calculate position";
		return false;
	}

	layoutPosition_t x2 = 0;
	layoutPosition_t y2 = 0;
	layoutPosition_t z2 = 0;
	layoutItemSize_t w2 = 0;
	layoutItemSize_t h2 = 0;

	Track* track = getTrack(trackID);
	if (track != nullptr)
	{
		z2 = track->posZ;
		ret = datamodel::LayoutItem::MapPosition(track->posX, track->posY, Width1, track->height, track->rotation, x2, y2, w2, h2);
		if (ret == false)
		{
			result = "Unable to calculate position";
			return false;
		}
	}

	for(layoutPosition_t ix = x1; ix < x1 + w1; ++ix)
	{
		for(layoutPosition_t iy = y1; iy < y1 + h1; ++iy)
		{
			ret = (ix >= x2 && ix < x2 + w2 && iy >= y2 && iy < y2 + h2 && z1 == z2);
			if (ret == true)
			{
				continue;
			}

			ret = CheckPositionFree(ix, iy, z1, result);
			if (ret == false)
			{
				return false;
			}
		}
	}
	return true;
}

bool Manager::trackSave(const trackID_t trackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t height, const layoutRotation_t rotation, const trackType_t type, string& result)
{
	Track* track;
	if (!CheckTrackPosition(trackID, posX, posY, posZ, height, rotation, result))
	{
		result.append(" Unable to ");
		result.append(trackID == TrackNone ? "add" : "move");
		result.append(" track.");
		return false;
	}
	{
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
			track->height = height;
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
			track = new Track(newTrackID, name, posX, posY, posZ, height, rotation, type);
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
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->trackSettings(track->objectID, name, posX, posY, posZ, height, track->Rotation());
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

	if (storage)
	{
		storage->deleteTrack(trackID);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->trackDelete(track->objectID, track->Name());
	}
	delete track;
	return true;
}

/***************************
* Switch                   *
***************************/

void Manager::SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state)
{
	Switch* mySwitch = getSwitch(switchID);
	if (mySwitch == nullptr)
	{
		return;
	}
	mySwitch->state = state;

	this->SwitchState(controlType, switchID, state, mySwitch->IsInverted(), true);

	delayedCall->Switch(controlType, switchID, state, mySwitch->IsInverted(), mySwitch->timeout);
}

void Manager::SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool inverted, const bool on)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		switchState_t tempState = (control.first >= ControlIdFirstHardware ? (state != inverted) : state);
		control.second->SwitchState(controlType, switchID, tempState, on);
	}
}

Switch* Manager::getSwitch(const switchID_t switchID) const
{
	std::lock_guard<std::mutex> Guard(switchMutex);
	if (switches.count(switchID) != 1)
	{
		return NULL;
	}
	return switches.at(switchID);
}

const std::string& Manager::getSwitchName(const switchID_t switchID) const
{
	if (switches.count(switchID) != 1)
	{
		return unknownSwitch;
	}
	return switches.at(switchID)->name;
}

bool Manager::CheckSwitchPosition(const switchID_t switchID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const
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
	if (!checkControlAccessoryProtocolAddress(controlID, protocol, address, result))
	{
		return false;
	}

	if (!CheckSwitchPosition(switchID, posX, posY, posZ) && !CheckPositionFree(posX, posY, posZ, Width1, Height1, rotation, result))
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

	if (storage)
	{
		storage->deleteSwitch(switchID);
	}

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->switchDelete(switchID, mySwitch->Name());
	}
	delete mySwitch;
	return true;
}

const map<string,datamodel::Switch*> Manager::switchListByName() const
{
	map<string,datamodel::Switch*> out;
	std::lock_guard<std::mutex> Guard(switchMutex);
	for(auto mySwitch : switches)
	{
		out[mySwitch.second->name] = mySwitch.second;
	}
	return out;
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

void Manager::executeStreet(const streetID_t streetID)
{
	Street* street = getStreet(streetID);
	if (street == nullptr)
	{
		return;
	}
	street->Execute();
}

void Manager::executeStreetInParallel(const streetID_t streetID)
{
	Street* street = getStreet(streetID);
	if (street == nullptr)
	{
		return;
	}
	std::async(std::launch::async, Street::ExecuteStatic, street);
}

Street* Manager::getStreet(const streetID_t streetID) const
{
	std::lock_guard<std::mutex> Guard(streetMutex);
	if (streets.count(streetID) != 1)
	{
		return NULL;
	}
	return streets.at(streetID);
}

const string& Manager::getStreetName(const streetID_t streetID) const
{
	if (streets.count(streetID) != 1)
	{
		return unknownStreet;
	}
	return streets.at(streetID)->name;
}

bool Manager::CheckStreetPosition(const streetID_t streetID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const
{
	Street* street = getStreet(streetID);
	if (street == nullptr)
	{
		return false;
	}

	if (street->visible == VisibleNo)
	{
		return false;
	}

	return (street->posX == posX && street->posY == posY && street->posZ == posZ);
}

bool Manager::streetSave(const streetID_t streetID, const std::string& name, const delay_t delay, const std::vector<datamodel::Relation*>& relations, const visible_t visible, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const automode_t automode, const trackID_t fromTrack, const direction_t fromDirection, const trackID_t toTrack, const direction_t toDirection, const feedbackID_t feedbackID, string& result)
{

	if (visible && !CheckStreetPosition(streetID, posX, posY, posZ) && !CheckPositionFree(posX, posY, posZ, Width1, Height1, Rotation0, result))
	{
		result.append("Unable to ");
		result.append(streetID == StreetNone ? "add" : "move");
		result.append(" street.");
		return false;
	}

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
			street->Delay(delay);
			street->AssignRelations(relations);
			street->visible = visible;
			street->posX = posX;
			street->posY = posY;
			street->posZ = posZ;
			street->automode = automode;
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
			street = new Street(this, newStreetID, name, delay, relations, visible, posX, posY, posZ, automode, fromTrack, fromDirection, toTrack, toDirection, feedbackID);
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
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->streetSettings(street->objectID, name);
	}
	return true;
}

const map<string,datamodel::Street*> Manager::streetListByName() const
{
	map<string,datamodel::Street*> out;
	std::lock_guard<std::mutex> Guard(streetMutex);
	for(auto street : streets)
	{
		out[street.second->name] = street.second;
	}
	return out;
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

	if (storage)
	{
		storage->deleteStreet(streetID);
	}

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->streetDelete(streetID, street->Name());
	}
	delete street;
	return true;
}

Layer* Manager::GetLayer(const layerID_t layerID) const
{
	std::lock_guard<std::mutex> Guard(layerMutex);
	if (layers.count(layerID) != 1)
	{
		return NULL;
	}
	return layers.at(layerID);
}

const map<string,layerID_t> Manager::LayerListByName() const
{
	map<string,layerID_t> list;
	std::lock_guard<std::mutex> Guard(layerMutex);
	for (auto layer : layers)
	{
		list[layer.second->Name()] = layer.first;
	}
	return list;
}

const map<string,layerID_t> Manager::LayerListByNameWithFeedback() const
{
	map<string,layerID_t> list = LayerListByName();
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		if (!control.second->CanHandleFeedback())
		{
			continue;
		}
		list["Feedback at " + control.second->getName()] = -control.first;
	}
	return list;
}

bool Manager::LayerSave(const layerID_t layerID, const std::string&name, std::string& result)
{
	Layer* layer;
	{
		std::lock_guard<std::mutex> Guard(layerMutex);
		if (layers.count(layerID))
		{
			// update existing layer
			layer = layers.at(layerID);
			if (layer == nullptr)
			{
				result.assign("Layer does not exist");
				return false;
			}
			layer->name = name;
		}
		else
		{
			// create new street
			layerID_t newLayerID = 0;
			// get next streetID
			for (auto layer : layers)
			{
				if (layer.first > newLayerID)
				{
					newLayerID = layer.first;
				}
			}
			++newLayerID;
			layer = new Layer(newLayerID, name);
			if (layer == nullptr)
			{
				result.assign("Unable to allocate memory for layer");
				return false;
			}
			// save in map
			layers[newLayerID] = layer;
		}
	}
	// save in db
	if (storage)
	{
		storage->layer(*layer);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->layerSettings(layer->objectID, name);
	}
	return true;
}

bool Manager::LayerDelete(const layerID_t layerID)
{
	if (layerID == LayerUndeletable || layerID == LayerNone)
	{
		return false;
	}

	Layer* layer = nullptr;
	{
		std::lock_guard<std::mutex> Guard(layerMutex);
		if (layers.count(layerID) == 0)
		{
			return false;
		}

		layer = layers.at(layerID);
		layers.erase(layerID);
	}

	if (storage)
	{
		storage->deleteLayer(layerID);
	}

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->layerDelete(layerID, layer->Name());
	}
	delete layer;
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

	logger->Info("{0} ({1}) is now on track {2} ({3})", loco->name, loco->objectID, track->name, track->objectID);

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->locoIntoTrack(locoID, trackID);
	}
	return true;
}

bool Manager::locoRelease(const locoID_t locoID)
{
	LocoSpeed(ControlTypeInternal, locoID, MinSpeed);

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
	locoID_t locoID = feedback->GetLoco();
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

void Manager::StopAllLocosImmediately(const controlType_t controlType)
{
	for (auto loco : locos)
	{
		locoID_t locoId = loco.second->objectID;
		LocoSpeed(controlType, locoId, MinSpeed);
	}
}

/***************************
* Layout                   *
***************************/

bool Manager::CheckPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, string& result) const
{
	return CheckLayoutPositionFree(posX, posY, posZ, result, accessories, accessoryMutex)
		&& CheckLayoutPositionFree(posX, posY, posZ, result, tracks, trackMutex)
		&& CheckLayoutPositionFree(posX, posY, posZ, result, feedbacks, feedbackMutex)
		&& CheckLayoutPositionFree(posX, posY, posZ, result, switches, switchMutex)
		&& CheckLayoutPositionFree(posX, posY, posZ, result, streets, streetMutex)
		&& CheckLayoutPositionFree(posX, posY, posZ, result, feedbacks, feedbackMutex);
}

bool Manager::CheckPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t width, const layoutItemSize_t height, const layoutRotation_t rotation, string& result) const
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
	bool ret = datamodel::LayoutItem::MapPosition(posX, posY, width, height, rotation, x, y, w, h);
	if (ret == false)
	{
		return false;
	}
	for(layoutPosition_t ix = x; ix < x + w; ix++)
	{
		for(layoutPosition_t iy = y; iy < y + h; iy++)
		{
			bool ret = CheckPositionFree(ix, iy, z, result);
			if (ret == false)
			{
				return false;
			}
		}
	}
	return true;
}

template<class Type>
bool Manager::CheckLayoutPositionFree(const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, string& result, const map<objectID_t, Type*>& layoutVector, std::mutex& mutex) const
{
	std::lock_guard<std::mutex> Guard(mutex);
	for (auto layout : layoutVector)
	{
		if (layout.second->CheckPositionFree(posX, posY, posZ))
		{
			continue;
		}
		stringstream status;
		status << "Position " << static_cast<int>(posX) << "/" << static_cast<int>(posY) << "/" << static_cast<int>(posZ) << " is already used by " << layout.second->LayoutType() << " \"" << layout.second->name << "\".";
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
		bool ret;
		if (type == AddressTypeLoco)
		{
			ret = control->LocoProtocolSupported(protocol);
		}
		else
		{
			ret = control->AccessoryProtocolSupported(protocol);
		}
		if (!ret)
		{
			stringstream status;
			status << "Protocol " << static_cast<int>(protocol);
			if (protocol > ProtocolNone && protocol <= ProtocolEnd)
			{
				status << "/" << protocolSymbols[protocol];
			}
			status << " is not supported by control. Please use one of: ";
			std::vector<protocol_t> protocols;
			if (type == AddressTypeLoco)
			{
				control->GetLocoProtocols(protocols);
			}
			else
			{
				control->GetAccessoryProtocols(protocols);
			}
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

