#ifndef HARDWARE_HARDWARE_PARAMS_H
#define HARDWARE_HARDWARE_PARAMS_H

#include "datatypes.h"

class Manager;

namespace hardware {

	class HardwareParams {
		public:
			HardwareParams(controlID_t controlID, hardwareID_t hardwareID, std::string name, std::string ip);
			HardwareParams(Manager* manager, controlID_t controlID, hardwareID_t hardwareID, std::string name, std::string ip);
			Manager* manager;
			controlID_t controlID;
			hardwareID_t hardwareID;
			std::string name;
			std::string ip;
	};

} // namespace hardware
#endif // HARDWARE_HARDWARE_PARAMS_H

