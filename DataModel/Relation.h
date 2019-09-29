#pragma once

#include <map>
#include <string>

#include "DataModel/Accessory.h"
#include "DataModel/LockableItem.h"
#include "DataModel/Serializable.h"
#include "DataTypes.h"

class Manager;

namespace DataModel
{
	class Relation : protected Serializable, public LockableItem
	{
		public:
			Relation(Manager* manager,
				const objectType_t objectType1,
				const objectID_t objectID1,
				const objectType_t objectType2,
				const objectID_t objectID2,
				const priority_t priority,
				const accessoryState_t accessoryState)
			:	manager(manager),
				objectType1(objectType1),
			 	objectID1(objectID1),
				objectType2(objectType2),
				objectID2(objectID2),
				priority(priority),
				accessoryState(accessoryState)
			{}

			Relation(Manager* manager,
				const std::string& serialized)
			:	manager(manager),
				accessoryState(DataModel::Accessory::AccessoryStateOff)
			{
				Deserialize(serialized);
			}

			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;

			objectType_t ObjectType2() { return objectType2; }
			objectID_t ObjectID2() { return objectID2; }
			priority_t Priority() { return priority; }
			accessoryState_t AccessoryState() { return accessoryState; }
			bool Reserve(const locoID_t locoID) override;
			bool Lock(const locoID_t locoID) override;
			bool Release(const locoID_t locoID) override;
			bool Execute(const delay_t delay);

		private:
			LockableItem* GetObject2();

			Manager* manager;
			objectType_t objectType1;
			objectID_t objectID1;
			objectType_t objectType2;
			objectID_t objectID2;
			priority_t priority;
			accessoryState_t accessoryState;
	};
} // namespace DataModel

