#ifndef HARDWARE_PARAMS_H
#define HARDWARE_PARAMS_H

namespace hardware {

	struct HardwareParams {
			hardwareID_t hardwareID;
			controlID_t controlID;
			std::string name;
			std::string ip;
	};

} // namespace hardwarev
#endif // HARDWARE_PARAMS_H

