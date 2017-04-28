
#include <map>
#include <sstream>

#include "street.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	Street::Street(streetID_t streetID, std::string name) :
		streetID(streetID),
		name(name) {
	}

	Street::Street(const std::string& serialized) {
		deserialize(serialized);
	}

	std::string Street::serialize() const {
		stringstream ss;
		ss << "relationType=Street;streetID=" << (int)streetID << ";name=" << name;
		return ss.str();
	}

	bool Street::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		if (arguments.count("relationType") && arguments.at("relationType").compare("Street") == 0) {
			if (arguments.count("streetID")) streetID = stoi(arguments.at("streetID"));
			if (arguments.count("name")) name = arguments.at("name");
			return true;
		}
		return false;
	}

} // namespace datamodel

