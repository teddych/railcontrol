#include <dlfcn.h>              // dl*
#include <sstream>

#include "datatypes.h"
#include "hardware/HardwareHandler.h"
#include "util.h"

using std::string;

namespace hardware {

	HardwareHandler::HardwareHandler(Manager& manager, const HardwareParams* params) :
		CommandInterface(ControlTypeHardware),
		manager(manager),
		createHardware(nullptr),
		destroyHardware(nullptr),
		instance(nullptr),
		params(params)
	{
		hardwareType_t type = params->hardwareType;

		// FIXME: if the same hardware library is loaded twice
		// FIXME: the clean up does not work correctly
		// FIXME: the second unload will creash

		// generate symbol and library names
		char* error;
		string& symbol = hardwareSymbols[type];
		std::stringstream ss;
		ss << "hardware/" << symbol << ".so";

		void* dlhandle = manager.hardwareLibraryGet(type);
		if (dlhandle == nullptr) {
			// open dynamic library
			dlhandle = dlopen(ss.str().c_str(), RTLD_LAZY);
			if (!dlhandle) {
				xlog("Can not open library: %s", dlerror());
				return;
			}
			xlog("Hardware library %s loaded", symbol.c_str());
			if (!manager.hardwareLibraryAdd(type, dlhandle)) {
				xlog("Unable to store library address");
				return;
			}
		}

		// look for symbol create_*
		ss.str(std::string());
		ss << "create_" << symbol;
		const char* s = ss.str().c_str();
		createHardware_t* new_create_hardware = (createHardware_t*)dlsym(dlhandle, s);
		error = dlerror();
		if (error) {
			xlog("Unable to find symbol %s: %s", s, error);
			return;
		}

		// look for symbol destroy_*
		ss.str(std::string());
		ss << "destroy_" << symbol;
		s = ss.str().c_str();
		destroyHardware_t* new_destroy_hardware = (destroyHardware_t*)dlsym(dlhandle, ss.str().c_str());
		error = dlerror();
		if (error) {
			xlog("Unable to find symbol %s: %s", s, error);
			return;
		}

		// register  valid symbols
		createHardware = new_create_hardware;
		destroyHardware = new_destroy_hardware;

		// start control
		if (createHardware) {
			instance = createHardware(params);
		}

		return;
	}

	HardwareHandler::~HardwareHandler() {
		// stop control
		if (instance) {
			destroyHardware(instance);
			instance = nullptr;
		}

		hardwareType_t type = params->hardwareType;
		// close library
		if (manager.controlsOfHardwareType(type) > 1) {
			return;
		}
		void* dlhandle = manager.hardwareLibraryGet(type);
		if (dlhandle == nullptr) {
			return;
		}
		if (manager.hardwareLibraryRemove(type) == false) {
			return;
		}
		dlclose(dlhandle);
		xlog("Hardware library %s unloaded", hardwareSymbols[type].c_str());
	}

	const std::string HardwareHandler::getName() const {
		if (instance == nullptr) {
			return "Unknown, not running";
		}
		return instance->GetName();
	}

	void HardwareHandler::getProtocols(std::vector<protocol_t>& protocols) const {
		if (instance == nullptr) {
			protocols.push_back(ProtocolNone);
			return;
		}

		instance->GetProtocols(protocols);
	}

	bool HardwareHandler::protocolSupported(protocol_t protocol) const {
		if (instance == nullptr) {
			return false;
		}
		return instance->ProtocolSupported(protocol);
	}

	void HardwareHandler::booster(const controlType_t managerID, const boosterStatus_t status) {
		if (managerID == ControlTypeHardware || instance == nullptr) {
			return;
		}
		instance->Booster(status);
	}

	void HardwareHandler::locoSpeed(const controlType_t managerID, const locoID_t locoID, const speed_t speed) {
		if (managerID == ControlTypeHardware || instance == nullptr) {
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.locoProtocolAddress(locoID, controlID, protocol, address);
		if (controlID != getControlID()) {
			return;
		}
		instance->LocoSpeed(protocol, address, speed);
	}

	void HardwareHandler::locoDirection(const controlType_t managerID, const locoID_t locoID, const direction_t direction) {
		if (managerID == ControlTypeHardware || instance == nullptr) {
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.locoProtocolAddress(locoID, controlID, protocol, address);
		if (controlID != getControlID()) {
			return;
		}
		instance->LocoDirection(protocol, address, direction);
	}

	void HardwareHandler::locoFunction(const controlType_t managerID, const locoID_t locoID, const function_t function, const bool on) {
		if (managerID == ControlTypeHardware || instance == nullptr) {
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.locoProtocolAddress(locoID, controlID, protocol, address);
		if (controlID != getControlID()) {
			return;
		}
		instance->LocoFunction(protocol, address, function, on);
	}

	void HardwareHandler::accessory(const controlType_t managerID, const accessoryID_t accessoryID, const accessoryState_t state, const bool on)
	{
		if (managerID == ControlTypeHardware || instance == nullptr)
		{
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.accessoryProtocolAddress(accessoryID, controlID, protocol, address);
		if (controlID != getControlID()) {
			return;
		}
		instance->Accessory(protocol, address, state, on);
	}

	void HardwareHandler::handleSwitch(const controlType_t managerID, const switchID_t switchID, const switchState_t state) {
		if (managerID == ControlTypeHardware || instance == nullptr) {
			return;
		}
		controlID_t controlID = 0;
		protocol_t protocol = ProtocolNone;
		address_t address = AddressNone;
		manager.switchProtocolAddress(switchID, controlID, protocol, address);
		if (controlID != getControlID()) {
			return;
		}
		instance->Accessory(protocol, address, state, true);
	}

} // namespace hardware
