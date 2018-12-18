#include <map>
#include <sstream>
#include <string>

#include "datamodel/object.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	Object::Object(const objectID_t objectID, const string& name)
	:	objectID(objectID),
		name(name)
	{
	};

	std::string Object::serialize() const
	{
		stringstream ss;
		ss << "objectID=" << (int) objectID << ";name=" << name;
		return ss.str();
	}

	bool Object::deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		parseArguments(serialized, arguments);
		return deserialize(arguments);
	}

	bool Object::deserialize(const map<string, string>& arguments)
	{
		objectID = GetIntegerMapEntry(arguments, "objectID", ObjectNone);
		name = GetStringMapEntry(arguments, "name");
		return true;
	}

} // namespace datamodel

