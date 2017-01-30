#include <sstream>

#include "../util.h"
#include "virtual.h"

namespace hardware {

	// create_virt and destroy_virt are used to instantiate
	// and delete the command station in main program

	// create instance of virtual
	extern "C" Virtual* create_virtual(struct Params &params) {
		return new Virtual(params);
	}

	// delete instance of virtual
	extern "C" void destroy_virtual(Virtual* virt) {
		delete(virt);
	}


  Virtual::Virtual(struct Params &params) {
		std::stringstream ss;
		ss << "Virtual Command Station / " << params.name;
		name = ss.str();
	}
	// return the name
	std::string Virtual::getName() const {
		return name;
	}

	std::string Virtual::locoSpeed(protocol_t protocol, address_t address, speed_t speed) {
		std::stringstream ss;
		ss << "Setting speed of loco " << protocol << "/" << address << " to speed " << speed;
		return ss.str();
	}

} // namespace
