#include "switch.h"

namespace datamodel {

	Switch::Switch(switchID_t switchID, std::string name, controlID_t controlID, protocol_t protocol, address_t address, switchType_t type, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		Accessory(switchID, name, controlID, protocol, address, type, x, y, z) {
	}

} // namespace datamodel

