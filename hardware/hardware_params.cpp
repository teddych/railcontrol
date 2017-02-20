#include "hardware_params.h"

namespace hardware {
	HardwareParams::HardwareParams(controlID_t controlID, hardwareID_t hardwareID, std::string name, std::string ip) :
		controlID(controlID),
		hardwareID(hardwareID),
		name(name),
		ip(ip) {
	}

	HardwareParams::HardwareParams(Manager* manager, controlID_t controlID, hardwareID_t hardwareID, std::string name, std::string ip) :
		manager(manager),
		controlID(controlID),
		hardwareID(hardwareID),
		name(name),
		ip(ip) {
	}
} // namespace hardware
