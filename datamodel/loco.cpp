#include "loco.h"

namespace datamodel {

	Loco::Loco(locoID_t locoID, std::string name, controlID_t controlID, protocol_t protocol, address_t address) :
		locoID(locoID),
		name(name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		speed(0) {
	}

} // namespace datamodel

