
#ifndef AMALGAMATION
#include <dlfcn.h>              // dl*
#endif

#include <sstream>

#include "datatypes.h"
#include "Logger/Logger.h"
#include "hardware/HardwareHandler.h"
#include "hardware/cs2.h"
#include "hardware/m6051.h"
#include "hardware/rm485.h"
#include "hardware/virtual.h"
#include "util.h"

using std::string;

namespace hardware
{
	const std::string HardwareHandler::hardwareSymbols[] =
	{
		"none",
		"virtual",
		"cs2",
		"m6051",
		"rm485"
	};

	HardwareHandler::HardwareHandler(Manager& manager, const HardwareParams* params)
	:	CommandInterface(ControlTypeHardware),
		manager(manager),
		createHardware(nullptr),
		destroyHardware(nullptr),
		instance(nullptr),
		params(params)
	{
		hardwareType_t type = params->hardwareType;

#ifdef AMALGAMATION
		switch(type)
		{
			case HardwareTypeCS2:
				createHardware = (hardware::HardwareInterface* (*)(const hardware::HardwareParams*))(&create_cs2);
				destroyHardware = (void (*)(hardware::HardwareInterface*))(&destroy_cs2);
				break;

			case HardwareTypeVirtual:
				createHardware = (hardware::HardwareInterface* (*)(const hardware::HardwareParams*))(&create_virtual);
				destroyHardware = (void (*)(hardware::HardwareInterface*))(&destroy_virtual);
				break;


			case HardwareTypeM6051:
				createHardware = (hardware::HardwareInterface* (*)(const hardware::HardwareParams*))(&create_m6051);
				destroyHardware = (void (*)(hardware::HardwareInterface*))(&destroy_m6051);
				break;

			case HardwareTypeRM485:
				createHardware = (hardware::HardwareInterface* (*)(const hardware::HardwareParams*))(&create_rm485);
				destroyHardware = (void (*)(hardware::HardwareInterface*))(&destroy_rm485);
				break;

			default:
				createHardware = nullptr;
				destroyHardware = nullptr;
				break;
		}
#else
		// FIXME: if the same hardware library is loaded twice
		// FIXME: the clean up does not work correctly
		// FIXME: the second unload will crash

		// generate symbol and library names
		char* error;
		const string& symbol = hardwareSymbols[type];
		std::stringstream ss;
		ss << "hardware/" << symbol << ".so";

		Logger::Logger* logger = Logger::Logger::GetLogger("HardwareHandler");
		void* dlhandle = manager.hardwareLibraryGet(type);
		if (dlhandle == nullptr)
		{
			// open dynamic library
			dlhandle = dlopen(ss.str().c_str(), RTLD_LAZY);
			if (!dlhandle)
			{
				logger->Error("Can not open library: {0}", dlerror());
				return;
			}
			logger->Info("Hardware library {0} loaded", symbol);
			if (!manager.hardwareLibraryAdd(type, dlhandle))
			{
				logger->Error("Unable to store library address");
				return;
			}
		}

		// look for symbol create_*
		ss.str(std::string());
		ss << "create_" << symbol;
		string s = ss.str();
		createHardware_t* new_create_hardware = (createHardware_t*)dlsym(dlhandle, s.c_str());
		error = dlerror();
		if (error)
		{
			logger->Error("Unable to find symbol {0}: {1}", s, error);
			return;
		}

		// look for symbol destroy_*
		ss.str(std::string());
		ss << "destroy_" << symbol;
		s = ss.str();
		destroyHardware_t* new_destroy_hardware = (destroyHardware_t*)dlsym(dlhandle, s.c_str());
		error = dlerror();
		if (error)
		{
			logger->Error("Unable to find symbol {0}: {1}", s, error);
			return;
		}

		// register  valid symbols
		createHardware = new_create_hardware;
		destroyHardware = new_destroy_hardware;
#endif

		// start control
		if (createHardware)
		{
			instance = createHardware(params);
		}
	}

	HardwareHandler::~HardwareHandler()
	{
		// stop control
		if (instance)
		{
			destroyHardware(instance);
			instance = nullptr;
		}

#ifndef AMALGAMAGTION
		hardwareType_t type = params->hardwareType;
		// close library
		if (manager.controlsOfHardwareType(type) > 1)
		{
			return;
		}
		void* dlhandle = manager.hardwareLibraryGet(type);
		if (dlhandle == nullptr)
		{
			return;
		}
		if (manager.hardwareLibraryRemove(type) == false)
		{
			return;
		}
		dlclose(dlhandle);
		Logger::Logger::GetLogger("HardwareHandler")->Info("Hardware library {0} unloaded", hardwareSymbols[type]);
#endif
	}

	const std::string HardwareHandler::getName() const
	{
		if (instance == nullptr)
		{
			return "Unknown, not running";
		}
		return instance->GetName();
	}

	bool HardwareHandler::CanHandleLocos() const
	{
		if (instance == nullptr)
		{
			return false;
		}

		return instance->CanHandleLocos();
	}

	bool HardwareHandler::CanHandleAccessories() const
	{
		if (instance == nullptr)
		{
			return false;
		}

		return instance->CanHandleAccessories();
	}

	bool HardwareHandler::CanHandleFeedback() const
	{
		if (instance == nullptr)
		{
			return false;
		}

		return instance->CanHandleFeedback();
	}

	void HardwareHandler::GetLocoProtocols(std::vector<protocol_t>& protocols) const
	{
		if (instance == nullptr)
		{
			protocols.push_back(ProtocolNone);
			return;
		}

		instance->GetLocoProtocols(protocols);
	}

	bool HardwareHandler::LocoProtocolSupported(protocol_t protocol) const
	{
		if (instance == nullptr)
		{
			return false;
		}
		return instance->LocoProtocolSupported(protocol);
	}

	void HardwareHandler::GetAccessoryProtocols(std::vector<protocol_t>& protocols) const
	{
		if (instance == nullptr)
		{
			protocols.push_back(ProtocolNone);
			return;
		}

		instance->GetAccessoryProtocols(protocols);
	}

	bool HardwareHandler::AccessoryProtocolSupported(protocol_t protocol) const
	{
		if (instance == nullptr)
		{
			return false;
		}
		return instance->AccessoryProtocolSupported(protocol);
	}

	void HardwareHandler::GetArgumentTypes(std::map<unsigned char,argumentType_t>& argumentTypes) const
	{
		if (instance == nullptr)
		{
			return;
		}
		instance->GetArgumentTypes(argumentTypes);
	}

	void HardwareHandler::booster(const controlType_t controlType, const boosterStatus_t status)
	{
		if (controlType == ControlTypeHardware || instance == nullptr)
		{
			return;
		}
		instance->Booster(status);
	}

	void HardwareHandler::LocoSpeed(const controlType_t controlType, const locoID_t locoID, const locoSpeed_t speed)
	{
		if (controlType == ControlTypeHardware || instance == nullptr)
		{
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.locoProtocolAddress(locoID, controlID, protocol, address);
		if (controlID != getControlID())
		{
			return;
		}
		instance->LocoSpeed(protocol, address, speed);
	}

	void HardwareHandler::LocoDirection(const controlType_t controlType, const locoID_t locoID, const direction_t direction)
	{
		if (controlType == ControlTypeHardware || instance == nullptr)
		{
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.locoProtocolAddress(locoID, controlID, protocol, address);
		if (controlID != getControlID())
		{
			return;
		}
		instance->LocoDirection(protocol, address, direction);
	}

	void HardwareHandler::LocoFunction(const controlType_t controlType, const locoID_t locoID, const function_t function, const bool on)
	{
		if (controlType == ControlTypeHardware || instance == nullptr)
		{
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.locoProtocolAddress(locoID, controlID, protocol, address);
		if (controlID != getControlID())
		{
			return;
		}
		instance->LocoFunction(protocol, address, function, on);
	}

	void HardwareHandler::AccessoryState(const controlType_t controlType, const accessoryID_t accessoryID, const accessoryState_t state, const bool on)
	{
		if (controlType == ControlTypeHardware || instance == nullptr)
		{
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.accessoryProtocolAddress(accessoryID, controlID, protocol, address);
		if (controlID != getControlID())
		{
			return;
		}
		instance->Accessory(protocol, address, state, on);
	}

	void HardwareHandler::SwitchState(const controlType_t controlType, const switchID_t switchID, const switchState_t state, const bool on)
	{
		if (controlType == ControlTypeHardware || instance == nullptr)
		{
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.switchProtocolAddress(switchID, controlID, protocol, address);
		if (controlID != getControlID())
		{
			return;
		}
		instance->Accessory(protocol, address, state, on);
	}

} // namespace hardware
