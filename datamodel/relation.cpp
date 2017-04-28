#include <map>
#include <sstream>
#include <string>

#include "relation.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel {

	std::string Relation::serialize() const {
		stringstream ss;
		ss << ";relationType=" << (int)relationType << ";objectID1=" << (int)objectID1 << ";objectID2=" << (int)objectID2;
		return ss.str();
	}

	bool Relation::deserialize(const std::string& serialized) {
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		return deserialize(arguments);
	}

	bool Relation::deserialize(const map<string,string>& arguments) {
		if (arguments.count("relationType")) relationType = stoi(arguments.at("relationType"));
		if (arguments.count("objectID1")) objectID1 = stoi(arguments.at("objectID1"));
		if (arguments.count("objectID2")) objectID2 = stoi(arguments.at("objectID2"));
		return true;
	}

} // namespace datamodel

