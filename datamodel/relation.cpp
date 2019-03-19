#include <map>
#include <sstream>
#include <string>

#include "datamodel/relation.h"
#include "manager.h"

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
		objectType1 = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objectType1"));
		objectID1 = GetIntegerMapEntry(arguments, "objectID1");
		objectType2 = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objectType2"));
		objectID2 = GetIntegerMapEntry(arguments, "objectID2");
		priority = GetIntegerMapEntry(arguments, "priority");
		accessoryState = GetIntegerMapEntry(arguments, "accessoryState");
		lockState = static_cast<lockState_t>(GetIntegerMapEntry(arguments, "lockState", LockStateFree));
		return true;
	}

	bool Relation::Execute()
	{
		switch (objectType2)
		{
			case ObjectTypeAccessory:
				manager->AccessoryState(ControlTypeInternal, objectID2, accessoryState);
				return true;

			case ObjectTypeSwitch:
				manager->SwitchState(ControlTypeInternal, objectID2, accessoryState);
				return true;

			default:
				return false;
		}

	}

	bool Relation::Reserve(const locoID_t locoID)
	{
		if (lockState == LockStateFree)
		{
			return true;
		}
		switch (objectType2)
		{
			case ObjectTypeAccessory:
			{
				Accessory* accessory = manager->GetAccessory(objectID2);
				if (accessory == nullptr)
				{
					return false;
				}
				return true; // FIXME: fix accessory locking
			}

			case ObjectTypeSwitch:
			{
				Switch* mySwitch = manager->GetSwitch(objectID2);
				if (mySwitch == nullptr)
				{
					return false;
				}
				return true; // FIXME: fix switch locking
			}

			default:
				return false;
		}
	}

	bool Relation::Lock(const locoID_t locoID)
	{
		return true;
	}

	bool Relation::Release(const locoID_t locoID)
	{
		return true;
	}
} // namespace datamodel

