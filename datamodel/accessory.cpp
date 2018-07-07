#include <map>
#include <sstream>

#include "accessory.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Accessory::Accessory(const accessoryID_t accessoryID,
		const std::string& name,
		const layoutPosition_t x,
		const layoutPosition_t y,
		const layoutPosition_t z,
		const layoutRotation_t rotation,
		const controlID_t controlID,
		const protocol_t protocol,
		const address_t address,
		const accessoryType_t type,
		const accessoryState_t state,
		const accessoryTimeout_t timeout) :
		LayoutItem(accessoryID, name, x, y, z, Width1, Height1, rotation),
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
		ss << "objectType=Accessory;" << serializeWithoutType();
		return ss.str();
	}

	std::string Accessory::serializeWithoutType() const {
		stringstream ss;
		ss << LayoutItem::serialize() << ";controlID=" << (int)controlID << ";protocol=" << (int)protocol << ";address=" << (int)address << ";type=" << (int)type << ";state=" << (int)state << ";timeout=" << (int)timeout;
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
		if (arguments.count("controlID")) controlID = stoi(arguments.at("controlID"));
		if (arguments.count("protocol")) protocol = stoi(arguments.at("protocol"));
		if (arguments.count("address")) address = stoi(arguments.at("address"));
		if (arguments.count("type")) type = stoi(arguments.at("type"));
		if (arguments.count("state")) state = stoi(arguments.at("state"));
		if (arguments.count("timeout")) timeout = stoi(arguments.at("timeout"));
		return true;
	}
} // namespace datamodel
