#include <sstream>

#include "../util.h"
#include "virtual.h"

namespace hardware {

	// create_virt and destroy_virt are used to instantiate
	// and delete the command station in main program

	// create instance of virtual
	extern "C" Virtual* create_virtual(const HardwareParams* params) {
		return new Virtual(params);
	}

	// delete instance of virtual
	extern "C" void destroy_virtual(Virtual* virt) {
		delete(virt);
	}


  Virtual::Virtual(const HardwareParams* params) {
		std::stringstream ss;
		ss << "Virtual Command Station / " << params->name;
		name = ss.str();
	}

	// GO-command (turn on booster)
  void Virtual::go() {
		xlog("Turning virtual booster on");
  }

	// Stop-command (turn off booster)
  void Virtual::stop() {
		xlog("Turning virtual booster off");
  }

	// return the name
	std::string Virtual::getName() const {
		return name;
	}

	// set loco speed
	void Virtual::locoSpeed(const protocol_t& protocol, const address_t& address, const speed_t& speed) {
		xlog("Setting speed of virtual loco %i/%i to speed %i", protocol, address, speed);
	}

	// set the direction of a loco
	void Virtual::locoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction) {
		xlog("Setting direction of virtual loco %i/%i to %s", protocol, address, direction ? "forward" : "reverse");
	}

	// set loco function
	void Virtual::locoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on) {
		xlog("Setting f%i of virtual loco %i/%i to \"%s\"", (int)function, (int)protocol, (int)address, on ? "on" : "off");
	}

	// accessory command
	void Virtual::accessory(const protocol_t protocol, const address_t address, const accessoryState_t state) {
		char* stateText;
		switch (state) {
			case 0:
				"red on";
				break;
			case 1:
				"red off";
				break;
			case 2:
				"green on";
				break;
			case 3:
				"green off";
				break;
		}
		xlog("Setting state of virtual accessory %i/%i to \"%s\"", (int)protocol, (int)address, stateText);
	}

} // namespace
