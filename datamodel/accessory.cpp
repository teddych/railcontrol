#include "accessory.h"

namespace datamodel {

	Accessory::Accessory(accessoryID_t accessoryID, std::string name, controlID_t controlID, protocol_t protocol, address_t address, accessoryType_t type, accessoryState_t state, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		LayoutItem(ROTATION_0, /* width */ 1, /* height */ 1, x, y, z),
		accessoryID(accessoryID),
		name(name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		type(type),
		state(state) {
	}

} // namespace datamodel

