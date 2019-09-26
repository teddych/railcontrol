#include <map>

#include "DataModel/Switch.h"
#include "Utils/Utils.h"

using std::map;
using std::stringstream;
using std::string;

namespace DataModel
{
	std::string Switch::Serialize() const
	{
		return "objectType=Switch;" + Accessory::SerializeWithoutType();
	}

	bool Switch::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		string objectType = Utils::Utils::GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Switch") != 0)
		{
			return false;
		}

		return Accessory::Deserialize(arguments);
	}
} // namespace DataModel

