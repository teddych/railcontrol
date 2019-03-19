#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "serializable.h"

class Manager;

namespace datamodel
{
	class Relation : protected Serializable
	{
		public:
			Relation(Manager* manager,
				const objectType_t objectType1,
				const objectID_t objectID1,
				const objectType_t objectType2,
				const objectID_t objectID2,
				const priority_t priority,
				const accessoryState_t accessoryState,
				const lockState_t lockState)
			:	manager(manager),
				objectType1(objectType1),
			 	objectID1(objectID1),
				objectType2(objectType2),
				objectID2(objectID2),
				priority(priority),
				accessoryState(accessoryState),
				lockState(lockState)
			{}

			Relation(Manager* manager,
				const std::string& serialized)
			:	manager(manager),
				accessoryState(AccessoryStateOff),
				lockState(LockStateFree)
			{
				Deserialize(serialized);
			}

			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;

			objectType_t ObjectType2() { return objectType2; }
			objectID_t ObjectID2() { return objectID2; }
			priority_t Priority() { return priority; }
			accessoryState_t AccessoryState() { return accessoryState; }
			lockState_t LockState() { return lockState; }
			bool Reserve(const locoID_t locoID);
			bool Lock(const locoID_t locoID);
			bool Release(const locoID_t locoID);
			bool Execute();

		private:
			Manager* manager;
			objectType_t objectType1;
			objectID_t objectID1;
			objectType_t objectType2;
			objectID_t objectID2;
			priority_t priority;
			accessoryState_t accessoryState;
			lockState_t lockState;
	};
} // namespace datamodel

