#include "hardware_params.h"

namespace hardware {
	HardwareParams::HardwareParams(controlID_t controlID, hardwareType_t hardwareType, std::string name, std::string ip) :
		controlID(controlID),
		hardwareType(hardwareType),
		name(name),
		ip(ip) {
	}

	HardwareParams::HardwareParams(Manager* manager, controlID_t controlID, hardwareType_t hardwareType, std::string name, std::string ip) :
		manager(manager),
		controlID(controlID),
		hardwareType(hardwareType),
		name(name),
		ip(ip) {
	}
} // namespace hardware
