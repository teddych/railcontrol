#include <map>
#include <sstream>

#include "accessory.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Accessory::Accessory(accessoryID_t accessoryID, std::string name, controlID_t controlID, protocol_t protocol, address_t address, accessoryType_t type, accessoryState_t state, accessoryTimeout_t timeout, layoutPosition_t x, layoutPosition_t y, layoutPosition_t z) :
		LayoutItem(ROTATION_0, /* width */ 1, /* height */ 1, x, y, z),
		accessoryID(accessoryID),
		name(name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		type(type),
		state(state),
		timeout(timeout) {
	}

	Accessory::Accessory(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Accessory::serialize() const {
		stringstream ss;
		ss << "objectType=Accessory;accessoryID=" << (int)accessoryID << ";name=" << name << ";controlID=" << (int)controlID << ";protocol=" << (int)protocol << ";address=" << (int)address << ";type=" << (int)type << ";state=" << (int)state << ";timeout=" << (int)timeout << LayoutItem::serialize();
		return ss.str();
	}

	bool Accessory::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Accessory") == 0) {
			return deserialize(arguments);
		}
		return false;
	}

	bool Accessory::deserialize(const map<string,string>& arguments) {
		LayoutItem::deserialize(arguments);
		if (arguments.count("accessoryID")) accessoryID = stoi(arguments.at("accessoryID"));
		if (arguments.count("name")) name = arguments.at("name");
		if (arguments.count("controlID")) controlID = stoi(arguments.at("controlID"));
		if (arguments.count("protocol")) protocol = stoi(arguments.at("protocol"));
		if (arguments.count("address")) address = stoi(arguments.at("address"));
		if (arguments.count("type")) type = stoi(arguments.at("type"));
		if (arguments.count("state")) state = stoi(arguments.at("state"));
		if (arguments.count("timeout")) timeout = stoi(arguments.at("timeout"));
		return true;
	}


	void Accessory::getAccessoryTexts(const accessoryState_t state, unsigned char& color, unsigned char& on, char*& colorText, char*& onText) {
		// calculate color as number
		color = state >> 1;
		// calculate color as text
		switch (color) {
			case ACCESSORY_COLOR_RED:
				colorText = (char*)"red";
				break;
			case ACCESSORY_COLOR_GREEN:
				colorText = (char*)"green";
				break;
			case ACCESSORY_COLOR_YELLOW:
				colorText = (char*)"yellow";
				break;
			case ACCESSORY_COLOR_WHITE:
				colorText = (char*)"white";
				break;
			default:
				colorText = (char*)"unknown";
		}
		// calculate on as number
		on = state & 0x01;
		// calculate on as text
		switch (on) {
			case ACCESSORY_STATE_OFF:
				onText = (char*)"off";
				break;
			case ACCESSORY_STATE_ON:
				onText = (char*)"on";
				break;
			default:
				onText = (char*)"unknown";
		}
	}

} // namespace datamodel

