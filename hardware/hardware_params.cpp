#include "hardware_params.h"

namespace hardware {
	HardwareParams::HardwareParams(controlID_t controlID, hardwareID_t hardwareID, std::string name, std::string ip) :
		controlID(controlID),
		hardwareID(hardwareID),
		name(name),
		ip(ip) {
	}
} // namespace hardware
