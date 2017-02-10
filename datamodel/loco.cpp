#include "loco.h"

namespace datamodel {

	Loco::Loco(locoID_t locoID) :
		locoID(locoID) {
	}

	Loco::Loco(locoID_t locoID, std::string name, protocol_t protocol, address_t address) :
		locoID(locoID),
		name(name),
		protocol(protocol),
		address(address),
		speed(0) {
	}

	Loco::~Loco() {
	}

} // namespace Loco

