#include "switch.h"

namespace datamodel {

	Switch::Switch(switchID_t switchID,
	               std::string name,
								 controlID_t controlID,
								 protocol_t protocol,
								 address_t address,
								 switchType_t type,
								 switchState_t state,
								 layoutRotation_t rotation,
								 layoutPosition_t x,
								 layoutPosition_t y,
								 layoutPosition_t z) :
		Accessory(switchID, name, controlID, protocol, address, type, state << 1, /* timeout ms*/ 200, x, y, z),
		switchID(switchID),
		rotation(rotation) {
	}

	/*
	Switch::Switch(const std::string serialized) {
		deserialize(serialized);
	}
	*/

	std::string Switch::serialize() const {
		return "";
	}

	bool Switch::deserialize(const std::string serialized) {
		return true;
	}

	void Switch::getTexts(const switchState_t state, char*& stateText) {
		switch (state >> 1) {
			case SWITCH_STRAIGHT:
				stateText = (char*)"straight";
				break;
			case SWITCH_TURNOUT:
				stateText = (char*)"turnout";
				break;
			default:
				stateText = (char*)"unknown";
		}
	}

} // namespace datamodel

