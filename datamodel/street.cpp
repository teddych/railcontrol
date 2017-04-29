
#include <map>
#include <sstream>

#include "street.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Street::Street(const streetID_t streetID, const std::string& name, const blockID_t fromBlock, const blockID_t toBlock) :
		Object(streetID, name),
		fromBlock(fromBlock),
		toBlock(toBlock) {
	}

	Street::Street(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Street::serialize() const {
		stringstream ss;
		ss << "relationType=Street;" << Object::serialize() << ";fromBlock=" << (int)fromBlock << ";toBlock=" << (int)toBlock;
		return ss.str();
	}

	bool Street::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("relationType") && arguments.at("relationType").compare("Street") == 0) {
			Object::deserialize(arguments);
			if (arguments.count("fromBlock")) fromBlock = stoi(arguments.at("fromBlock"));
			if (arguments.count("toBlock")) toBlock = stoi(arguments.at("toBlock"));
			return true;
		}
		return false;
	}

} // namespace datamodel

