#include <map>
#include <sstream>
#include <string>

#include "datamodel/object.h"
#include "Utils/Utils.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Object::Serialize() const
	{
		stringstream ss;
		ss << "objectID=" << (int) objectID << ";name=" << name;
		return ss.str();
	}

	bool Object::Deserialize(const std::string& serialized)
	{
		map<string, string> arguments;
		ParseArguments(serialized, arguments);
		return Deserialize(arguments);
	}

	bool Object::Deserialize(const map<string, string>& arguments)
	{
		objectID = GetIntegerMapEntry(arguments, "objectID", ObjectNone);
		name = GetStringMapEntry(arguments, "name");
		return true;
	}

} // namespace datamodel

