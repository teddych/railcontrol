/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2019 Dominik (Teddy) Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#include <map>
#include <sstream>
#include <string>

#include "DataModel/Relation.h"
#include "Manager.h"
#include "Utils/Utils.h"

using std::map;
using std::stringstream;
using std::string;

namespace DataModel
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
		objectType1 = static_cast<objectType_t>(Utils::Utils::GetIntegerMapEntry(arguments, "objectType1"));
		objectID1 = Utils::Utils::GetIntegerMapEntry(arguments, "objectID1");
		objectType2 = static_cast<objectType_t>(Utils::Utils::GetIntegerMapEntry(arguments, "objectType2"));
		objectID2 = Utils::Utils::GetIntegerMapEntry(arguments, "objectID2");
		priority = Utils::Utils::GetIntegerMapEntry(arguments, "priority");
		accessoryState = Utils::Utils::GetIntegerMapEntry(arguments, "accessoryState");
		return true;
	}

	bool Relation::Execute(const delay_t delay)
	{
		switch (objectType2)
		{
			case ObjectTypeAccessory:
			{
				bool ret = manager->AccessoryState(ControlTypeInternal, objectID2, accessoryState, true);
				if (ret == false)
				{
					return false;
				}
				break;
			}

			case ObjectTypeSwitch:
			{
				bool ret = manager->SwitchState(ControlTypeInternal, objectID2, accessoryState, true);
				if (ret == false)
				{
					return false;
				}
				break;
			}

			case ObjectTypeSignal:
			{
				bool ret = manager->SignalState(ControlTypeInternal, objectID2, accessoryState, true);
				if (ret == false)
				{
					return false;
				}
				break;
			}

			case ObjectTypeTrack:
				return true;

			default:
				return false;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(delay));
		return true;
	}

	LockableItem* Relation::GetObject2()
	{
		switch (objectType2)
		{
			case ObjectTypeAccessory:
				return manager->GetAccessory(objectID2);

			case ObjectTypeSwitch:
				return manager->GetSwitch(objectID2);

			case ObjectTypeTrack:
				return manager->GetTrack(objectID2);

			case ObjectTypeSignal:
				return manager->GetSignal(objectID2);

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
} // namespace DataModel

