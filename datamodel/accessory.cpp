#include "accessory.h"

namespace datamodel {

	Accessory::Accessory(accessoryID_t accessoryID, std::string name, controlID_t controlID, protocol_t protocol, address_t address, accessoryType_t type, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		accessoryID(accessoryID),
		name(name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		type(type),
		state(0) {

		height = 1;
		width = 1;
		rotation = ROTATION_0;
		posX = x;
		posY = y;
		posZ = z;
	}

} // namespace datamodel

