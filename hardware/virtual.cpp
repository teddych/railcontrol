#include "virtual.h"

namespace hardware {

	// create_virt and destroy_virt are used to instantiate
	// and delete the command station in main program

	// create instance of virtual
	extern "C" Virtual* create_virtual() {
		return new Virtual();
	}

	// delete instance of virtual
	extern "C" void destroy_virtual(Virtual* virt) {
		delete(virt);
	}

	// return the name
	std::string Virtual::name() const {
		return "Virtual Command Station";
	}

} // namespace
