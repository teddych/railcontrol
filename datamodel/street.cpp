
#include <map>
#include <sstream>

#include "street.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Street::Street(const streetID_t streetID, const std::string& name, const blockID_t fromBlock, const direction_t fromDirection, const blockID_t toBlock, const direction_t toDirection) :
		Object(streetID, name),
		fromBlock(fromBlock),
		fromDirection(fromDirection),
		toBlock(toBlock),
		toDirection(toDirection),
		locoID(LOCO_NONE) {
	}

	Street::Street(const std::string& serialized) :
		locoID(LOCO_NONE) {
		deserialize(serialized);
	}

	std::string Street::serialize() const {
		stringstream ss;
		ss << "relationType=Street;" << Object::serialize() << ";fromBlock=" << (int)fromBlock << ";fromDirection=" << (int)fromDirection << ";toBlock=" << (int)toBlock << ";toDirection=" << (int)toDirection;
		return ss.str();
	}

	bool Street::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("relationType") && arguments.at("relationType").compare("Street") == 0) {
			Object::deserialize(arguments);
			if (arguments.count("fromBlock")) fromBlock = stoi(arguments.at("fromBlock"));
			if (arguments.count("fromDirection")) fromDirection = (bool)stoi(arguments.at("fromDirection"));
			if (arguments.count("toBlock")) toBlock = stoi(arguments.at("toBlock"));
			if (arguments.count("toDirection")) toDirection = (bool)stoi(arguments.at("toDirection"));
			return true;
		}
		return false;
	}

	bool Street::fromBlockDirection(blockID_t blockID, direction_t direction) {
		return (fromBlock == blockID && fromDirection == direction);
	}

} // namespace datamodel

