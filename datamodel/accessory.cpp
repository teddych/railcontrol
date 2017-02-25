#include "accessory.h"

namespace datamodel {

	Accessory::Accessory(accessoryID_t accessoryID, std::string name, controlID_t controlID, protocol_t protocol, address_t address, accessoryType_t type) :
		accessoryID(accessoryID),
		name(name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		type(type),
		state(0) {
	}

} // namespace datamodel

