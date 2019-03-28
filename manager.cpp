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
using std::vector;
using storage::StorageHandler;
using storage::StorageParams;
using webserver::WebServer;

Manager::Manager(Config& config)
:	logger(Logger::Logger::GetLogger("Manager")),
 	boosterState(BoosterStop),
	storage(nullptr),
 	delayedCall(new DelayedCall(*this)),
	defaultAccessoryDuration(DefaultAccessoryDuration),
	autoAddFeedback(false),
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

	defaultAccessoryDuration = Util::StringToInteger(storage->GetSetting("DefaultAccessoryDuration"));
	autoAddFeedback = Util::StringToBool(storage->GetSetting("AutoAddFeedback"));


	controls[ControlIdConsole] = new ConsoleServer(*this, config.getValue("consoleport", 2222));
	controls[ControlIdWebserver] = new WebServer(*this, config.getValue("webserverport", 80));

	storage->AllHardwareParams(hardwareParams);
	for (auto hardwareParam : hardwareParams)
	{
		hardwareParam.second->manager = this;
		controls[hardwareParam.second->controlID] = new HardwareHandler(*this, hardwareParam.second);
		logger->Info("Loaded control {0}: {1}", hardwareParam.first, hardwareParam.second->name);
	}

	storage->AllLayers(layers);
	for (auto layer : layers)
	{
		logger->Info("Loaded layer {0}: {1}", layer.second->objectID, layer.second->Name());
	}
	if (layers.count(LayerUndeletable) != 1)
	{
		string result;
		bool initLayer0 = LayerSave(0, "Layer 1", result);
		if (initLayer0 == false)
		{
			logger->Error("Unable to add initial layer 1");
		}
	}

	storage->AllLocos(locos);
	for (auto loco : locos)
	{
		logger->Info("Loaded loco {0}: {1}", loco.second->objectID, loco.second->name);
	}

	storage->AllAccessories(accessories);
	for (auto accessory : accessories)
	{
		logger->Info("Loaded accessory {0}: {1}", accessory.second->objectID, accessory.second->name);
	}

	storage->AllFeedbacks(feedbacks);
	for (auto feedback : feedbacks)
	{
		logger->Info("Loaded feedback {0}: {1}", feedback.second->objectID, feedback.second->name);
	}

	storage->AllTracks(tracks);
	for (auto track : tracks)
	{
		logger->Info("Loaded track {0}: {1}", track.second->objectID, track.second->name);
	}

	storage->AllSwitches(switches);
	for (auto mySwitch : switches)
	{
		logger->Info("Loaded switch {0}: {1}", mySwitch.second->objectID, mySwitch.second->name);
	}

	storage->AllStreets(streets);
	for (auto street : streets)
	{
		logger->Info("Loaded street {0}: {1}", street.second->objectID, street.second->name);
	}
	// FIXME: load locos into tracks
}

Manager::~Manager()
{
	while (!LocoStopAll())
	{
		sleep(1);
	}

	{
		std::lock_guard<std::mutex> Guard2(controlMutex);
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
	}

	if (storage != nullptr)
	{
		storage->StartTransaction();
	}

	DeleteAllMapEntries(streets, streetMutex);
	DeleteAllMapEntries(switches, switchMutex, storage);
	DeleteAllMapEntries(accessories, accessoryMutex, storage);
	DeleteAllMapEntries(feedbacks, feedbackMutex, storage);
	DeleteAllMapEntries(tracks, trackMutex, storage);
	DeleteAllMapEntries(locos, locoMutex, storage);
	DeleteAllMapEntries(layers, layerMutex);

	delete delayedCall;
	delayedCall = nullptr;

	if (storage == nullptr)
	{
		return;
	}

	storage->CommitTransaction();
	delete storage;
	storage = nullptr;
}

/***************************
* Booster                  *
***************************/

void Manager::Booster(const controlType_t controlType, const boosterState_t state)
{
	boosterState = state;
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->Booster(controlType, state);
	}
}

/***************************
* Control                  *
***************************/

const std::map<hardwareType_t,string> Manager::HardwareListNames()
{
	std::map<hardwareType_t,string> hardwareList;
	hardwareList[HardwareTypeM6051] = "Märklin Interface 6051/6051";
	hardwareList[HardwareTypeCS2] = "Märklin Control Station 2 (CS2)";
	hardwareList[HardwareTypeRM485] = "RM485";
	hardwareList[HardwareTypeVirtual] = "Virtual Command Station (no Hardware)";
	return hardwareList;
}

bool Manager::ControlSave(const controlID_t& controlID,
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

	HardwareParams* params = GetHardware(controlID);
	if (params != nullptr)
	{
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

		controls[newControlID] = new HardwareHandler(*this, params);
		hardwareParams[newControlID] = params;
	}
	if (storage)
	{
		storage->Save(*params);
	}
	return true;
}

bool Manager::ControlDelete(controlID_t controlID)
{
	HardwareParams* params = nullptr;
	{
		std::lock_guard<std::mutex> Guard(hardwareMutex);
		if (controlID < ControlIdFirstHardware || hardwareParams.count(controlID) != 1)
		{
			return false;
		}

		params = hardwareParams.at(controlID);
		if (params == nullptr)
		{
			return false;
		}
		hardwareParams.erase(controlID);
		delete params;
	}
	{
		std::lock_guard<std::mutex> Guard(controlMutex);
		if (controls.count(controlID) != 1)
		{
			return false;
		}
		ControlInterface* control = controls.at(controlID);
		controls.erase(controlID);
		delete control;
	}

	if (storage)
	{
		storage->DeleteHardwareParams(controlID);
	}
	return true;
}

HardwareParams* Manager::GetHardware(controlID_t controlID)
{
	std::lock_guard<std::mutex> Guard(hardwareMutex);
	if (hardwareParams.count(controlID) != 1)
	{
		return nullptr;
	}
	return hardwareParams.at(controlID);
}

unsigned int Manager::ControlsOfHardwareType(const hardwareType_t hardwareType)
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

bool Manager::HardwareLibraryAdd(const hardwareType_t hardwareType, void* libraryHandle)
{
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) == 1)
	{
		return false;
	}
	hardwareLibraries[hardwareType] = libraryHandle;
	return true;
}

void* Manager::HardwareLibraryGet(const hardwareType_t hardwareType) const
{
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) != 1)
	{
		return nullptr;
	}
	return hardwareLibraries.at(hardwareType);
}

bool Manager::HardwareLibraryRemove(const hardwareType_t hardwareType)
{
	std::lock_guard<std::mutex> Guard(hardwareLibrariesMutex);
	if (hardwareLibraries.count(hardwareType) != 1)
	{
		return false;
	}
	hardwareLibraries.erase(hardwareType);
	return true;
}

const ControlInterface* Manager::GetControl(const controlID_t controlID) const
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	if (controls.count(controlID) != 1)
	{
		return nullptr;
	}
	return controls.at(controlID);
}

const std::string Manager::GetControlName(const controlID_t controlID)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	if (controls.count(controlID) != 1)
	{
		return unknownControl;
	}
	ControlInterface* control = controls.at(controlID);
	return control->Name();
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
		ControlInterface* c = controls.at(hardware.second->controlID);
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
		ControlInterface* c = controls.at(hardware.second->controlID);
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
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		if (control.second->ControlType() != ControlTypeHardware || control.second->CanHandleFeedback() == false)
		{
			continue;
		}
		ret[control.first] = control.second->Name();
	}
	return ret;
}

const map<string,hardware::HardwareParams*> Manager::ControlListByName() const
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
		const ControlInterface* control = GetControl(controlID);
		if (control == nullptr || control->ControlType() != ControlTypeHardware)
		{
			ret[protocolSymbols[ProtocolNone]] = ProtocolNone;
			return ret;
		}

		const HardwareHandler* hardware = static_cast<const HardwareHandler*>(control);
		if (hardware->ControlID() != controlID)
		{
			ret[protocolSymbols[ProtocolNone]] = ProtocolNone;
			return ret;
		}

		std::vector<protocol_t> protocols;
		if (type == AddressTypeLoco)
		{
			hardware->LocoProtocols(protocols);
		}
		else
		{
			hardware->AccessoryProtocols(protocols);
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

	const ControlInterface* control = controls.at(controlID);
	control->ArgumentTypes(ret);
	return ret;
}

/***************************
* Loco                     *
***************************/

datamodel::Loco* Manager::GetLoco(const locoID_t locoID) const
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) != 1)
	{
		return nullptr;
	}
	return locos.at(locoID);
}

const std::string& Manager::GetLocoName(const locoID_t locoID) const
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) != 1)
	{
		return unknownLoco;
	}
	return locos.at(locoID)->Name();
}

const map<string,locoID_t> Manager::LocoListFree() const
{
	map<string,locoID_t> out;
	std::lock_guard<std::mutex> Guard(locoMutex);
	for(auto loco : locos)
	{
		if (loco.second->GetTrack() == TrackNone)
		{
			out[loco.second->Name()] = loco.second->objectID;
		}
	}
	return out;
}

const map<string,datamodel::Loco*> Manager::LocoListByName() const
{
	map<string,datamodel::Loco*> out;
	std::lock_guard<std::mutex> Guard(locoMutex);
	for(auto loco : locos)
	{
		out[loco.second->Name()] = loco.second;
	}
	return out;
}

bool Manager::LocoSave(const locoID_t locoID,
	const string& name,
	const controlID_t controlID,
	const protocol_t protocol,
	const address_t address,
	const function_t nr,
	const locoSpeed_t maxSpeed,
	const locoSpeed_t travelSpeed,
	const locoSpeed_t reducedSpeed,
	const locoSpeed_t creepSpeed,
	string& result)
{
	if (!CheckControlLocoProtocolAddress(controlID, protocol, address, result))
	{
		return false;
	}

	Loco* loco = GetLoco(locoID);
	if (loco != nullptr)
	{
		// update existing loco
		loco->name = name;
		loco->controlID = controlID;
		loco->protocol = protocol;
		loco->address = address;
		loco->SetNrOfFunctions(nr);
		loco->SetMaxSpeed(maxSpeed);
		loco->SetTravelSpeed(travelSpeed);
		loco->SetReducedSpeed(reducedSpeed);
		loco->SetCreepSpeed(creepSpeed);
	}
	else
	{
		// create new loco
		std::lock_guard<std::mutex> Guard(locoMutex);
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
		loco = new Loco(this, newLocoID, name, controlID, protocol, address, nr, maxSpeed, travelSpeed, reducedSpeed, creepSpeed);
		if (loco == nullptr)
		{
			result.assign("Unable to allocate memory for loco");
			return false;
		}
		// save in map
		locos[newLocoID] = loco;
	}

	// save in db
	if (storage)
	{
		storage->Save(*loco);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoSettings(loco->objectID, name);
	}
	return true;
}

bool Manager::LocoDelete(const locoID_t locoID)
{
	Loco* loco = nullptr;
	{
		std::lock_guard<std::mutex> Guard(locoMutex);
		if (locoID == LocoNone || locos.count(locoID) != 1)
		{
			return false;
		}

		loco = locos.at(locoID);
		if (loco->IsInUse())
		{
			return false;
		}

		locos.erase(locoID);
	}

	if (storage)
	{
		storage->DeleteLoco(locoID);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoDelete(loco->objectID, loco->Name());
	}
	delete loco;
	return true;
}

bool Manager::LocoProtocolAddress(const locoID_t locoID, controlID_t& controlID, protocol_t& protocol, address_t& address) const
{
	std::lock_guard<std::mutex> Guard(locoMutex);
	if (locos.count(locoID) != 1)
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
	Loco* loco = GetLoco(locoID);
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
	Loco* loco = GetLoco(locoID);
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
	Loco* loco = GetLoco(locoID);
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
	Loco* loco = GetLoco(locoID);
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
		AccessoryState(controlType, accessoryID, state, true);
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
		SwitchState(controlType, switchID, state, true);
		return;
	}

	// FIXME: add code for signals
}

void Manager::AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool force)
{
	Accessory* accessory = GetAccessory(accessoryID);
	if (accessory == nullptr)
	{
		return;
	}

	if (force == false && accessory->IsInUse())
	{
		logger->Warning("{0} is locked", accessory->Name());
		return;
	}

	accessory->state = state;

	this->AccessoryState(controlType, accessoryID, state, accessory->IsInverted(), true);

	delayedCall->Accessory(controlType, accessoryID, state, accessory->IsInverted(), accessory->duration);
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

Accessory* Manager::GetAccessory(const accessoryID_t accessoryID) const
{
	std::lock_guard<std::mutex> Guard(accessoryMutex);
	if (accessories.count(accessoryID) != 1)
	{
		return nullptr;
	}
	return accessories.at(accessoryID);
}

const std::string& Manager::GetAccessoryName(const accessoryID_t accessoryID) const
{
	std::lock_guard<std::mutex> Guard(accessoryMutex);
	if (accessories.count(accessoryID) != 1)
	{
		return unknownAccessory;
	}
	return accessories.at(accessoryID)->name;
}

bool Manager::CheckAccessoryPosition(const accessoryID_t accessoryID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const
{
	Accessory* accessory = GetAccessory(accessoryID);
	if (accessory == nullptr)
	{
		return false;
	}

	return (accessory->posX == posX && accessory->posY == posY && accessory->posZ == posZ);
}

bool Manager::AccessorySave(const accessoryID_t accessoryID, const string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const protocol_t protocol, const address_t address, const accessoryType_t type, const accessoryDuration_t duration, const bool inverted, string& result)
{
	if (!CheckControlAccessoryProtocolAddress(controlID, protocol, address, result))
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

	Accessory* accessory = GetAccessory(accessoryID);
	if (accessory != nullptr)
	{
		// update existing accessory
		accessory->name = name;
		accessory->posX = posX;
		accessory->posY = posY;
		accessory->posZ = posZ;
		accessory->controlID = controlID;
		accessory->protocol = protocol;
		accessory->address = address;
		accessory->type = type;
		accessory->duration = duration;
		accessory->Inverted(inverted);
	}
	else
	{
		// create new accessory
		std::lock_guard<std::mutex> Guard(accessoryMutex);
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
		accessory = new Accessory(newAccessoryID, name, posX, posY, posZ, Rotation0, controlID, protocol, address, type, duration, inverted);
		if (accessory == nullptr)
		{
			result.assign("Unable to allocate memory for accessory");
			return false;
		}
		// save in map
		accessories[newAccessoryID] = accessory;
	}

	// save in db
	if (storage)
	{
		storage->Save(*accessory);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->AccessorySettings(accessory->objectID, name);
	}
	return true;
}

const map<string,datamodel::Accessory*> Manager::AccessoryListByName() const
{
	map<string,datamodel::Accessory*> out;
	std::lock_guard<std::mutex> Guard(accessoryMutex);
	for(auto accessory : accessories)
	{
		out[accessory.second->name] = accessory.second;
	}
	return out;
}

bool Manager::AccessoryDelete(const accessoryID_t accessoryID)
{
	Accessory* accessory = nullptr;
	{
		std::lock_guard<std::mutex> Guard(accessoryMutex);
		if (accessoryID == AccessoryNone || accessories.count(accessoryID) != 1)
		{
			return false;
		}

		accessory = accessories.at(accessoryID);
		accessories.erase(accessoryID);
	}

	if (storage)
	{
		storage->DeleteAccessory(accessoryID);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->AccessoryDelete(accessoryID, accessory->Name());
	}
	delete accessory;
	return true;
}

bool Manager::AccessoryRelease(const accessoryID_t accessoryID)
{
	Accessory* accessory = GetAccessory(accessoryID);
	if (accessory == nullptr)
	{
		return false;
	}
	locoID_t locoID = accessory->GetLoco();
	return accessory->Release(locoID);
}

bool Manager::AccessoryProtocolAddress(const accessoryID_t accessoryID, controlID_t& controlID, protocol_t& protocol, address_t& address) const
{
	if (accessories.count(accessoryID) != 1)
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
	if (feedbackID == FeedbackNone && GetAutoAddFeedback() == true)
	{
		string name = "Feedback auto added " + std::to_string(controlID) + "/" + std::to_string(pin);
		logger->Info("Adding feedback {0}", name);
		string result;
		feedbackID = FeedbackSave(FeedbackNone, name, VisibleNo, 0, 0, 0, controlID, pin, false, result);
	}

	return FeedbackState(controlType, feedbackID, state);
}

void Manager::FeedbackState(const controlType_t controlType, const feedbackID_t feedbackID, const feedbackState_t state)
{
	Feedback* feedback = GetFeedback(feedbackID);
	if (feedback == nullptr)
	{
		return;
	}
	logger->Info("Feedback {0} is now {1}", feedback->Name(), (state ? "on" : "off"));
	{
		std::lock_guard<std::mutex> Guard(controlMutex);
		for (auto control : controls)
		{
			control.second->FeedbackState(controlType, feedback->Name(), feedbackID, state);
		}
	}
	feedback->SetState(state);
}

datamodel::Feedback* Manager::GetFeedback(feedbackID_t feedbackID) const
{
	std::lock_guard<std::mutex> Guard(feedbackMutex);
	if (feedbacks.count(feedbackID) != 1)
	{
		return nullptr;
	}
	return feedbacks.at(feedbackID);
}

const std::string& Manager::GetFeedbackName(const feedbackID_t feedbackID) const
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
	Feedback* feedback = GetFeedback(feedbackID);
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

feedbackID_t Manager::FeedbackSave(const feedbackID_t feedbackID, const std::string& name, const visible_t visible, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const controlID_t controlID, const feedbackPin_t pin, const bool inverted, string& result)
{
	if (visible && !CheckFeedbackPosition(feedbackID, posX, posY, posZ) && !CheckPositionFree(posX, posY, posZ, Width1, Height1, Rotation0, result))
	{
		result.append(" Unable to ");
		result.append(feedbackID == FeedbackNone ? "add" : "move");
		result.append(" feedback.");
		return FeedbackNone;
	}

	Feedback* feedback = GetFeedback(feedbackID);
	if (feedback != nullptr)
	{
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
		std::lock_guard<std::mutex> Guard(feedbackMutex);
		feedbackID_t newFeedbackID = 0;
		// get next feedbackID
		for (auto feedback : feedbacks)
		{
			if (feedback.first > newFeedbackID)
			{
				newFeedbackID = feedback.first;
			}
		}
		++newFeedbackID;
		feedback = new Feedback(this, newFeedbackID, name, visible, posX, posY, posZ, controlID, pin, inverted);
		if (feedback == nullptr)
		{
			result.assign("Unable to allocate memory for feedback");
			return FeedbackNone;
		}
		// save in map
		feedbacks[newFeedbackID] = feedback;
	}

	// save in db
	if (storage)
	{
		storage->Save(*feedback);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->FeedbackSettings(feedback->objectID, name);
	}
	return feedback->objectID;
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

const map<string,feedbackID_t> Manager::FeedbacksOfTrack(const trackID_t trackID) const
{
	map<string,feedbackID_t> out;
	Track* track = GetTrack(trackID);
	if (track == nullptr)
	{
		return out;
	}
	vector<feedbackID_t> feedbacksOfTrack = track->GetFeedbacks();
	for (auto feedbackID : feedbacksOfTrack)
	{
		Feedback* feedback = GetFeedback(feedbackID);
		if (feedback == nullptr)
		{
			continue;
		}
		out[feedback->Name()] = feedbackID;
	}
	return out;
}

bool Manager::FeedbackDelete(const feedbackID_t feedbackID)
{
	Feedback* feedback = nullptr;
	{
		std::lock_guard<std::mutex> Guard(feedbackMutex);
		if (feedbackID == FeedbackNone || feedbacks.count(feedbackID) != 1)
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
		storage->DeleteFeedback(feedbackID);
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

Track* Manager::GetTrack(const trackID_t trackID) const
{
	std::lock_guard<std::mutex> Guard(trackMutex);
	if (tracks.count(trackID) != 1)
	{
		return nullptr;
	}
	return tracks.at(trackID);
}

const std::string& Manager::GetTrackName(const trackID_t trackID) const
{
	if (tracks.count(trackID) != 1)
	{
		return unknownTrack;
	}
	return tracks.at(trackID)->name;
}

const map<string,datamodel::Track*> Manager::TrackListByName() const
{
	map<string,datamodel::Track*> out;
	std::lock_guard<std::mutex> Guard(trackMutex);
	for(auto track : tracks)
	{
		out[track.second->name] = track.second;
	}
	return out;
}

const map<string,trackID_t> Manager::TrackListIdByName() const
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

	Track* track = GetTrack(trackID);
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

const std::vector<feedbackID_t> Manager::CleanupAndCheckFeedbacks(trackID_t trackID, std::vector<feedbackID_t>& newFeedbacks)
{
	{
		std::lock_guard<std::mutex> feedbackGuard(feedbackMutex);
		for (auto feedback : feedbacks)
		{
			if (feedback.second->GetTrack() == trackID)
			{
				feedback.second->SetTrack(TrackNone);
			}
		}
	}

	std::vector<feedbackID_t> checkedFeedbacks;
	for (auto feedbackID : newFeedbacks)
	{
		Feedback* feedback = GetFeedback(feedbackID);
		if (feedback == nullptr)
		{
			continue;
		}
		if (feedback->GetTrack() != TrackNone)
		{
			continue;
		}
		checkedFeedbacks.push_back(feedbackID);
		feedback->SetTrack(trackID);
	}
	return checkedFeedbacks;
}

trackID_t Manager::TrackSave(const trackID_t trackID, const std::string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutItemSize_t height, const layoutRotation_t rotation, const trackType_t type, std::vector<feedbackID_t> newFeedbacks, string& result)
{
	if (!CheckTrackPosition(trackID, posX, posY, posZ, height, rotation, result))
	{
		result.append(" Unable to ");
		result.append(trackID == TrackNone ? "add" : "move");
		result.append(" track.");
		return TrackNone;
	}

	Track* track = GetTrack(trackID);
	if (track != nullptr)
	{
		// update existing track
		track->name = name;
		track->height = height;
		track->rotation = rotation;
		track->posX = posX;
		track->posY = posY;
		track->posZ = posZ;
		track->Type(type);
		track->Feedbacks(CleanupAndCheckFeedbacks(trackID, newFeedbacks));
	}
	else
	{
		// create new track
		std::lock_guard<std::mutex> trackGuard(trackMutex);
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
		track = new Track(this, newTrackID, name, posX, posY, posZ, height, rotation, type, CleanupAndCheckFeedbacks(trackID, newFeedbacks));
		if (track == nullptr)
		{
			result.assign("Unable to allocate memory for track");
			return TrackNone;
		}
		// save in map
		tracks[newTrackID] = track;
	}

	// save in db
	if (storage)
	{
		storage->Save(*track);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->TrackSettings(track->objectID, name);
	}
	return track->objectID;
}

bool Manager::TrackDelete(const trackID_t trackID)
{
	Track* track = nullptr;
	{
		std::lock_guard<std::mutex> Guard(trackMutex);
		if (trackID == TrackNone || tracks.count(trackID) != 1)
		{
			return false;
		}

		track = tracks.at(trackID);
		if (track == nullptr || track->IsInUse())
		{
			return false;
		}

		tracks.erase(trackID);
	}

	if (storage)
	{
		storage->DeleteTrack(trackID);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->TrackDelete(track->objectID, track->Name());
	}
	delete track;
	return true;
}

/***************************
* Switch                   *
***************************/

void Manager::SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool force)
{
	Switch* mySwitch = GetSwitch(switchID);
	if (mySwitch == nullptr)
	{
		return;
	}

	if (force == false && mySwitch->IsInUse())
	{
		logger->Warning("{0} is locked", mySwitch->Name());
		return;
	}

	mySwitch->state = state;

	this->SwitchState(controlType, switchID, state, mySwitch->IsInverted(), true);

	delayedCall->Switch(controlType, switchID, state, mySwitch->IsInverted(), mySwitch->duration);
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

Switch* Manager::GetSwitch(const switchID_t switchID) const
{
	std::lock_guard<std::mutex> Guard(switchMutex);
	if (switches.count(switchID) != 1)
	{
		return nullptr;
	}
	return switches.at(switchID);
}

const std::string& Manager::GetSwitchName(const switchID_t switchID) const
{
	if (switches.count(switchID) != 1)
	{
		return unknownSwitch;
	}
	return switches.at(switchID)->name;
}

bool Manager::CheckSwitchPosition(const switchID_t switchID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const
{
	Switch* mySwitch = GetSwitch(switchID);
	if (mySwitch == nullptr)
	{
		return false;
	}

	return (mySwitch->posX == posX && mySwitch->posY == posY && mySwitch->posZ == posZ);
}

bool Manager::SwitchSave(const switchID_t switchID, const string& name, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const layoutRotation_t rotation, const controlID_t controlID, const protocol_t protocol, const address_t address, const switchType_t type, const switchDuration_t duration, const bool inverted, string& result)
{
	if (!CheckControlAccessoryProtocolAddress(controlID, protocol, address, result))
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

	Switch* mySwitch = GetSwitch(switchID);
	if (mySwitch != nullptr)
	{
		// update existing switch
		mySwitch->name = name;
		mySwitch->posX = posX;
		mySwitch->posY = posY;
		mySwitch->posZ = posZ;
		mySwitch->rotation = rotation;
		mySwitch->controlID = controlID;
		mySwitch->protocol = protocol;
		mySwitch->address = address;
		mySwitch->type = type;
		mySwitch->duration = duration;
		mySwitch->Inverted(inverted);
	}
	else
	{
		// create new switch
		std::lock_guard<std::mutex> Guard(switchMutex);
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
		mySwitch = new Switch(newSwitchID, name, posX, posY, posZ, rotation, controlID, protocol, address, type, duration, inverted);
		if (mySwitch == nullptr)
		{
			result.assign("Unable to allocate memory for switch");
			return false;
		}
		// save in map
		switches[newSwitchID] = mySwitch;
	}

	// save in db
	if (storage)
	{
		storage->Save(*mySwitch);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->SwitchSettings(mySwitch->objectID, name);
	}
	return true;
}

bool Manager::SwitchDelete(const switchID_t switchID)
{
	Switch* mySwitch = nullptr;
	{
		std::lock_guard<std::mutex> Guard(switchMutex);
		if (switchID == SwitchNone || switches.count(switchID) != 1)
		{
			return false;
		}

		mySwitch = switches.at(switchID);
		switches.erase(switchID);
	}

	if (storage)
	{
		storage->DeleteSwitch(switchID);
	}

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->SwitchDelete(switchID, mySwitch->Name());
	}
	delete mySwitch;
	return true;
}

const map<string,datamodel::Switch*> Manager::SwitchListByName() const
{
	map<string,datamodel::Switch*> out;
	std::lock_guard<std::mutex> Guard(switchMutex);
	for(auto mySwitch : switches)
	{
		out[mySwitch.second->name] = mySwitch.second;
	}
	return out;
}

bool Manager::SwitchProtocolAddress(const switchID_t switchID, controlID_t& controlID, protocol_t& protocol, address_t& address) const
{
	if (switches.count(switchID) != 1)
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

bool Manager::SwitchRelease(const streetID_t switchID)
{
	Switch* mySwitch = GetSwitch(switchID);
	if (mySwitch == nullptr)
	{
		return false;
	}
	locoID_t locoID = mySwitch->GetLoco();
	return mySwitch->Release(locoID);
}

/***************************
* Street                   *
***************************/

void Manager::ExecuteStreet(const streetID_t streetID)
{
	Street* street = GetStreet(streetID);
	if (street == nullptr)
	{
		return;
	}
	street->Execute();
}

void Manager::ExecuteStreetAsync(const streetID_t streetID)
{
	Street* street = GetStreet(streetID);
	if (street == nullptr)
	{
		return;
	}
	std::async(std::launch::async, Street::ExecuteStatic, street);
}

Street* Manager::GetStreet(const streetID_t streetID) const
{
	std::lock_guard<std::mutex> Guard(streetMutex);
	if (streets.count(streetID) != 1)
	{
		return nullptr;
	}
	return streets.at(streetID);
}

const string& Manager::GetStreetName(const streetID_t streetID) const
{
	std::lock_guard<std::mutex> Guard(streetMutex);
	if (streets.count(streetID) != 1)
	{
		return unknownStreet;
	}
	return streets.at(streetID)->name;
}

bool Manager::CheckStreetPosition(const streetID_t streetID, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ) const
{
	Street* street = GetStreet(streetID);
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

bool Manager::StreetSave(const streetID_t streetID, const std::string& name, const delay_t delay, const std::vector<datamodel::Relation*>& relations, const visible_t visible, const layoutPosition_t posX, const layoutPosition_t posY, const layoutPosition_t posZ, const automode_t automode, const trackID_t fromTrack, const direction_t fromDirection, const trackID_t toTrack, const direction_t toDirection, const feedbackID_t feedbackID, string& result)
{

	if (visible && !CheckStreetPosition(streetID, posX, posY, posZ) && !CheckPositionFree(posX, posY, posZ, Width1, Height1, Rotation0, result))
	{
		result.append("Unable to ");
		result.append(streetID == StreetNone ? "add" : "move");
		result.append(" street.");
		return false;
	}

	Street* street = GetStreet(streetID);
	if (street != nullptr)
	{
		// update existing street
		// remove street from old track
		Track* track = GetTrack(street->fromTrack);
		if (track != nullptr)
		{
			track->RemoveStreet(street);
			if (storage)
			{
				storage->Save(*track);
			}
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
		street->feedbackIdStop = feedbackID;
	}
	else
	{
		// create new street
		std::lock_guard<std::mutex> Guard(streetMutex);
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

	//Add new street
	Track* track = GetTrack(fromTrack);
	if (track != nullptr)
	{
		track->AddStreet(street);
		if (storage)
		{
			storage->Save(*track);
		}
	}
	// save in db
	if (storage)
	{
		storage->Save(*street);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->StreetSettings(street->objectID, name);
	}
	return true;
}

const map<string,datamodel::Street*> Manager::StreetListByName() const
{
	map<string,datamodel::Street*> out;
	std::lock_guard<std::mutex> Guard(streetMutex);
	for(auto street : streets)
	{
		out[street.second->name] = street.second;
	}
	return out;
}

bool Manager::StreetDelete(const streetID_t streetID)
{
	Street* street = nullptr;
	{
		std::lock_guard<std::mutex> Guard(streetMutex);
		if (streetID == StreetNone || streets.count(streetID) != 1)
		{
			return false;
		}

		street = streets.at(streetID);
		streets.erase(streetID);
	}

	if (storage)
	{
		storage->DeleteStreet(streetID);
	}

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->StreetDelete(streetID, street->Name());
	}
	delete street;
	return true;
}

Layer* Manager::GetLayer(const layerID_t layerID) const
{
	std::lock_guard<std::mutex> Guard(layerMutex);
	if (layers.count(layerID) != 1)
	{
		return nullptr;
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
		list["Feedback at " + control.second->Name()] = -control.first;
	}
	return list;
}

bool Manager::LayerSave(const layerID_t layerID, const std::string&name, std::string& result)
{
	Layer* layer = GetLayer(layerID);
	if (layer != nullptr)
	{
		// update existing layer
		layer->name = name;
	}
	else
	{
		// create new layer
		std::lock_guard<std::mutex> Guard(layerMutex);
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

	// save in db
	if (storage)
	{
		storage->Save(*layer);
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LayerSettings(layer->objectID, name);
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
		if (layers.count(layerID) != 1)
		{
			return false;
		}

		layer = layers.at(layerID);
		layers.erase(layerID);
	}

	if (storage)
	{
		storage->DeleteLayer(layerID);
	}

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LayerDelete(layerID, layer->Name());
	}
	delete layer;
	return true;
}

/***************************
* Automode                 *
***************************/

bool Manager::LocoIntoTrack(const locoID_t locoID, const trackID_t trackID)
{
	Track* track = GetTrack(trackID);
	if (track == nullptr)
	{
		return false;
	}

	Loco* loco = GetLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}

	bool reserved = track->Reserve(locoID);
	if (reserved == false)
	{
		return false;
	}

	reserved = loco->ToTrack(trackID);
	if (reserved == false)
	{
		track->Release(locoID);
		return false;
	}

	reserved = track->Lock(locoID);
	if (reserved == false)
	{
		loco->Release();
		track->Release(locoID);
		return false;
	}

	logger->Info("{0} ({1}) is now on track {2} ({3})", loco->name, loco->objectID, track->name, track->objectID);

	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoIntoTrack(locoID, trackID);
	}
	return true;
}

bool Manager::LocoRelease(const locoID_t locoID)
{
	Loco* loco = GetLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	trackID_t trackID = loco->GetTrack();
	streetID_t streetID = loco->GetStreet();
	bool ret = LocoReleaseInternal(locoID);
	ret &= StreetRelease(streetID);
	ret &= TrackRelease(trackID);
	return ret;
}

bool Manager::LocoReleaseInternal(const locoID_t locoID)
{
	LocoSpeed(ControlTypeInternal, locoID, MinSpeed);

	Loco* loco = GetLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	bool ret = loco->Release();
	if (ret == false)
	{
		return false;
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoRelease(locoID);
	}
	return true;
}

bool Manager::TrackRelease(const trackID_t trackID)
{
	Track* track = GetTrack(trackID);
	if (track == nullptr)
	{
		return false;
	}
	return TrackReleaseInternal(track);
}

bool Manager::TrackReleaseWithLoco(const trackID_t trackID)
{
	Track* track = GetTrack(trackID);
	if (track == nullptr)
	{
		return false;
	}
	locoID_t locoID = track->GetLoco();
	bool ret = LocoReleaseInternal(locoID);
	ret &= TrackReleaseInternal(track);
	return ret;
}

bool Manager::TrackReleaseInternal(Track* track)
{
	bool ret = track->Release(LocoNone);
	if (ret == false)
	{
		return false;
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->TrackState(ControlTypeInternal, track->Name(), track->objectID, track->FeedbackState(), "");
	}
	return true;
}
bool Manager::TrackStartLoco(const trackID_t trackID)
{
	Track* track = GetTrack(trackID);
	if (track == nullptr)
	{
		return false;
	}
	return LocoStart(track->GetLoco());
}

bool Manager::TrackStopLoco(const trackID_t trackID)
{
	Track* track = GetTrack(trackID);
	if (track == nullptr)
	{
		return false;
	}
	return LocoStop(track->GetLoco());
}

void Manager::TrackPublishState(const trackID_t trackID)
{
	Track* track = GetTrack(trackID);
	if (track == nullptr)
	{
		return;
	}
	TrackPublishState(track);
}

void Manager::TrackPublishState(const datamodel::Track* track)
{
	Loco* loco = GetLoco(track->GetLoco());
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->TrackState(ControlTypeInternal, track->Name(), track->objectID, track->FeedbackState(), loco == nullptr ? "" : loco->Name());
	}
}

bool Manager::StreetRelease(const streetID_t streetID)
{
	Street* street = GetStreet(streetID);
	if (street == nullptr)
	{
		return false;
	}
	locoID_t locoID = street->GetLoco();
	return street->Release(locoID);
}

/*
bool Manager::switchRelease(const switchID_t switchID) {
	Switch* mySwitch = GetSwitch(switchID);
	if (mySwitch == nullptr) {
		return false;
	}
	locoID_t locoID = mySwitch->getLoco();
	return mySwitch->release(locoID);
}
*/

bool Manager::LocoDestinationReached(const locoID_t locoID, const streetID_t streetID, const trackID_t trackID)
{
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoDestinationReached(locoID, streetID, trackID);
	}
	return true;
}

bool Manager::LocoStart(const locoID_t locoID)
{
	Loco* loco = GetLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	bool ret = loco->Start();
	if (ret == false)
	{
		return false;
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoStart(locoID);
	}
	return true;
}

bool Manager::LocoStop(const locoID_t locoID)
{
	Loco* loco = GetLoco(locoID);
	if (loco == nullptr)
	{
		return false;
	}
	bool ret = loco->Stop();
	if (ret == false)
	{
		return false;
	}
	std::lock_guard<std::mutex> Guard(controlMutex);
	for (auto control : controls)
	{
		control.second->LocoStop(locoID);
	}
	return true;
}

bool Manager::LocoStartAll()
{
	for (auto loco : locos)
	{
		bool ret = loco.second->Start();
		if (ret == false)
		{
			continue;
		}
		{
			std::lock_guard<std::mutex> Guard(controlMutex);
			for (auto control : controls)
			{
				control.second->LocoStart(loco.first);
			}
		}
	}
	return true;
}

bool Manager::LocoStopAll()
{
	bool ret1 = true;
	for (auto loco : locos)
	{
		if (!loco.second->IsInUse())
		{
			continue;
		}
		bool ret2 = loco.second->Stop();
		ret1 &= ret2;
		if (ret2 == false)
		{
			continue;
		}
		{
			std::lock_guard<std::mutex> Guard(controlMutex);
			for (auto control : controls)
			{
				control.second->LocoStop(loco.first);
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

bool Manager::CheckAddressLoco(const protocol_t protocol, const address_t address, string& result)
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

bool Manager::CheckAddressAccessory(const protocol_t protocol, const address_t address, string& result)
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

bool Manager::CheckControlProtocolAddress(const addressType_t type, const controlID_t controlID, const protocol_t protocol, const address_t address, string& result)
{
	{
		std::lock_guard<std::mutex> Guard(controlMutex);
		if (controlID < ControlIdFirstHardware || controls.count(controlID) != 1)
		{
			result.assign("Control does not exist");
			return false;
		}
		ControlInterface* control = controls.at(controlID);
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
				control->LocoProtocols(protocols);
			}
			else
			{
				control->AccessoryProtocols(protocols);
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
			return CheckAddressLoco(protocol, address, result);

		case AddressTypeAccessory:
			return CheckAddressAccessory(protocol, address, result);

		default:
			return false;
	}
}

bool Manager::SaveSettings(const accessoryDuration_t duration,
	const bool autoAddFeedback)
{
	this->defaultAccessoryDuration = duration;
	this->autoAddFeedback = autoAddFeedback;
	if (storage == nullptr)
	{
		return false;
	}
	storage->SaveSetting("DefaultAccessoryDuration", std::to_string(duration));
	storage->SaveSetting("AutoAddFeedback", std::to_string(autoAddFeedback));
	return true;
}
