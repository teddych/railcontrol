/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2020 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#pragma once

#include <map>
#include <string>

#include "DataModel/Accessory.h"
#include "DataModel/LockableItem.h"
#include "DataModel/Serializable.h"
#include "DataTypes.h"
#include "Logger/Logger.h"

class Manager;

namespace DataModel
{
	class Relation : protected Serializable, public LockableItem
	{
		public:
			enum type_t : unsigned char
			{
				TypeCalculate = 0,
				TypeStreetAtLock = 1,
				TypeStreetAtUnlock = 2,
				TypeLocoSlave = 10
			};

			Relation(Manager* manager,
				const objectType_t objectType1,
				const objectID_t objectID1,
				const objectType_t objectType2,
				const objectID_t objectID2,
				const type_t type,
				const priority_t priority,
				const unsigned char data)
			:	manager(manager),
				objectType1(objectType1),
			 	objectID1(objectID1),
				objectType2(objectType2),
				objectID2(objectID2),
				type(type),
				priority(priority),
				data(data)
			{}

			Relation(Manager* manager,
				const std::string& serialized)
			:	manager(manager),
				data(0)
			{
				Deserialize(serialized);
			}

			virtual ~Relation() {}

			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;

			objectType_t ObjectType2() const { return objectType2; }
			objectID_t ObjectID2() const { return objectID2; }
			type_t Type() const { return type; }
			priority_t Priority() const { return priority; }
			unsigned char GetState() const { return data; }

			bool Reserve(Logger::Logger* logger, const locoID_t locoID);
			bool Lock(Logger::Logger* logger, const locoID_t locoID);
			bool Release(const locoID_t locoID) override;
			bool Execute(Logger::Logger* logger, const delay_t delay);

		private:
			LockableItem* GetObject2();

			Manager* manager;
			objectType_t objectType1;
			objectID_t objectID1;
			objectType_t objectType2;
			objectID_t objectID2;
			type_t type;
			priority_t priority;
			unsigned char data;
	};
} // namespace DataModel

