#include <map>

#include "datamodel/switch.h"
#include "util.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Switch::Serialize() const
	{
		return "objectType=Switch;" + Accessory::SerializeWithoutType();
	}

	bool Switch::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		string objectType = GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Switch") != 0)
		{
			return false;
		}

		return Accessory::Deserialize(arguments);
	}
} // namespace datamodel

