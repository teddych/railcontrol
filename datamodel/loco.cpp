
#include <map>
#include <sstream>

#include "loco.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Loco::Loco(locoID_t locoID, std::string name, controlID_t controlID, protocol_t protocol, address_t address) :
		locoID(locoID),
		name(name),
		controlID(controlID),
		protocol(protocol),
		address(address),
		speed(0) {
	}

	Loco::Loco(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Loco::serialize() const {
		stringstream ss;
		ss << "objectType=Loco;locoID=" << (int)locoID << ";name=" << name << ";controlID=" << (int)controlID << ";protocol=" << (int)protocol << ";address=" << (int)address;
		return ss.str();
	}

	bool Loco::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("objectType") && arguments.at("objectType").compare("Loco") == 0) {
			if (arguments.count("locoID")) locoID = stoi(arguments.at("locoID"));
			if (arguments.count("name")) name = arguments.at("name");
			if (arguments.count("controlID")) controlID = stoi(arguments.at("controlID"));
			if (arguments.count("protocol")) protocol = stoi(arguments.at("protocol"));
			if (arguments.count("address")) address = stoi(arguments.at("address"));
			return true;
		}
		return false;
	}

} // namespace datamodel

