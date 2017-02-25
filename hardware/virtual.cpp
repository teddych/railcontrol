#include <sstream>

#include "manager.h"
#include "util.h"
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

	// turn booster on or off
  void Virtual::booster(const boosterStatus_t status) {
		if (status) xlog("Turning virtual booster on");
		else xlog("Turning virtual booster off");
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
		unsigned char color;
		unsigned char on;
		char* colorText;
		char* onText;
		Manager::getAccessoryTexts(state, color, on, colorText, onText);
		xlog("Setting state of virtual accessory %i/%i/%s to \"%s\"", (int)protocol, (int)address, colorText, onText);
	}

} // namespace
