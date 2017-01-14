#include <cstring>		//memset

#include "virt.h"

namespace hardware {

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

// have a look at hardware.h to find other methods that can be overwritten

} // namespace
