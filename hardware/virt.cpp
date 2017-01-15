#include "virt.h"

namespace hardware {

	// create_virt and destroy_virt are used to instantiate
	// and delete the command station in main program

	// create instance of virt
	extern "C" virt* create_virt() {
		return new virt();
	}

	// delete instance of virt
	extern "C" void destroy_virt(virt* virt) {
		delete(virt);
	}

	// return the name
	std::string virt::name() const {
		return "Virtual Command Station";
	}

} // namespace
