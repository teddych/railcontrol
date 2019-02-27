#include <map>
#include <sstream>
#include <string>

#include "datamodel/relation.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Relation::Serialize() const
	{
		stringstream ss;
		ss << "objectType1=" << static_cast<int>(objectType1)
			<< ";objectID1=" << objectID1
			<< ";objectType2=" << static_cast<int>(objectType2)
			<< ";objectID2=" << objectID2
			<< ";priority=" << static_cast<int>(priority)
			<< ";accessoryState=" << static_cast<int>(accessoryState)
			<< ";lockState=" << static_cast<int>(lockState);
		return ss.str();
	}

	bool Relation::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		parseArguments(serialized, arguments);
		return deserialize(arguments);
	}

	bool Relation::deserialize(const map<string,string>& arguments)
	{
		objectType1 = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objectType1"));
		objectID1 = GetIntegerMapEntry(arguments, "objectID1");
		objectType2 = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objectType2"));
		objectID2 = GetIntegerMapEntry(arguments, "objectID2");
		priority = GetIntegerMapEntry(arguments, "priority");
		accessoryState = GetIntegerMapEntry(arguments, "accessoryState");
		lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
		return true;
	}
} // namespace datamodel

