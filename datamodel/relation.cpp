#include <map>
#include <sstream>
#include <string>

#include "datamodel/relation.h"
#include "manager.h"
#include "util.h"

using std::map;
using std::stringstream;
using std::string;

namespace datamodel
{
	std::string Relation::Serialize() const
	{
		stringstream ss;
		ss << LockableItem::Serialize()
			<< ";objectType1=" << static_cast<int>(objectType1)
			<< ";objectID1=" << objectID1
			<< ";objectType2=" << static_cast<int>(objectType2)
			<< ";objectID2=" << objectID2
			<< ";priority=" << static_cast<int>(priority)
			<< ";accessoryState=" << static_cast<int>(accessoryState);
		return ss.str();
	}

	bool Relation::Deserialize(const std::string& serialized)
	{
		map<string,string> arguments;
		ParseArguments(serialized, arguments);
		LockableItem::Deserialize(arguments);
		objectType1 = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objectType1"));
		objectID1 = GetIntegerMapEntry(arguments, "objectID1");
		objectType2 = static_cast<objectType_t>(GetIntegerMapEntry(arguments, "objectType2"));
		objectID2 = GetIntegerMapEntry(arguments, "objectID2");
		priority = GetIntegerMapEntry(arguments, "priority");
		accessoryState = GetIntegerMapEntry(arguments, "accessoryState");
		return true;
	}

	bool Relation::Execute()
	{
		switch (objectType2)
		{
			case ObjectTypeAccessory:
				manager->AccessoryState(ControlTypeInternal, objectID2, accessoryState, true);
				return true;

			case ObjectTypeSwitch:
				manager->SwitchState(ControlTypeInternal, objectID2, accessoryState, true);
				return true;

			default:
				return false;
		}

	}

	LockableItem* Relation::GetObject2()
	{
		switch (objectType2)
		{
			case ObjectTypeAccessory:
				return manager->GetAccessory(objectID2);

			case ObjectTypeSwitch:
				return manager->GetSwitch(objectID2);

			default:
				return nullptr;
		}
	}

	bool Relation::Reserve(const locoID_t locoID)
	{
		bool ret = LockableItem::Reserve(locoID);
		if (ret == false)
		{
			return false;
		}

		LockableItem* object = GetObject2();
		if (object == nullptr)
		{
			LockableItem::Release(locoID);
			return false;
		}

		return object->Reserve(locoID);
	}

	bool Relation::Lock(const locoID_t locoID)
	{
		bool ret = LockableItem::Lock(locoID);
		if (ret == false)
		{
			return false;
		}

		LockableItem* object = GetObject2();
		if (object == nullptr)
		{
			LockableItem::Release(locoID);
			return false;
		}

		return object->Lock(locoID);
	}

	bool Relation::Release(const locoID_t locoID)
	{
		LockableItem* object = GetObject2();
		if (object != nullptr)
		{
			object->Release(locoID);
		}
		return LockableItem::Release(locoID);
	}
} // namespace datamodel

