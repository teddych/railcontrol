#include <map>
#include <sstream>

#include "switch.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Switch::Switch(switchID_t switchID,
		std::string name,
		layoutPosition_t x,
		layoutPosition_t y,
		layoutPosition_t z,
		layoutRotation_t rotation,
		controlID_t controlID,
		protocol_t protocol,
		address_t address,
		switchType_t type,
		switchState_t state,
		switchTimeout_t timeout) :
		Accessory(switchID, name, x, y, z, rotation, controlID, protocol, address, type, state << 1, timeout) {
	}

	Switch::Switch(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Switch::serialize() const {
		stringstream ss;
		ss << "objectType=Switch;" << Accessory::serializeWithoutType();
		return ss.str();
	}

	bool Switch::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Switch") == 0) {
			Accessory::deserialize(arguments);
			return true;
		}
		return false;
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

