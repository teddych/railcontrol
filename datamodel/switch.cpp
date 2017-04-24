#include <map>
#include <sstream>

#include "switch.h"

using std::map;
using std::stringstream;
using std::string;

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
		switchID(switchID) {
	}

	Switch::Switch(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Switch::serialize() const {
		stringstream ss;
		ss << "objectType=Switch;switchID=" << (int)switchID << Accessory::serialize();
		return ss.str();
	}

	bool Switch::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Switch") == 0) {
			Accessory::deserialize(arguments);
			if (arguments.count("switchID")) switchID = stoi(arguments.at("switchID"));
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

