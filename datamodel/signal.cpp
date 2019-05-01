#include <map>

#include "datamodel/signal.h"
#include "manager.h"
#include "Utils/Utils.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Signal::Serialize() const
	{
		return "objectType=Signal;" + Accessory::SerializeWithoutType();
	}

	bool Signal::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		string objectType = GetStringMapEntry(arguments, "objectType");
		if (objectType.compare("Signal") != 0)
		{
			return false;
		}

		return Accessory::Deserialize(arguments);
	}

	bool Signal::Release(const locoID_t locoID)
	{
		bool ret = LockableItem::Release(locoID);
		if (ret == false)
		{
			return false;
		}
		manager->SignalState(ControlTypeInternal, this, SignalStateRed, true);
		return true;
	}
} // namespace datamodel

