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

	Loco::Loco(const std::string serialized) {
		deserialize(serialized);
	}

	std::string Loco::serialize() const {
		return "";
	}

	bool Loco::deserialize(const std::string serialized) {
		return true;
	}

} // namespace datamodel

