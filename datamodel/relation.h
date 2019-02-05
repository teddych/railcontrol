#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "serializable.h"

namespace datamodel
{

	class Relation : protected Serializable
	{
		public:
			Relation(const objectType_t objectType1,
				const objectID_t objectID1,
				const objectType_t objectType2,
				const objectID_t objectID2,
				const priority_t priority,
				const accessoryState_t accessoryState,
				const lockState_t lockState)
			:	objectType1(objectType1),
				objectID1(objectID1),
				objectType2(objectType2),
				objectID2(objectID2),
				priority(priority),
				accessoryState(accessoryState),
				lockState(lockState)
			{}

			Relation(const std::string& serialized)
			:	accessoryState(AccessoryStateOff),
				lockState(LockStateFree)
			{
				deserialize(serialized);
			}

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;

			objectType_t ObjectType2() { return objectType2; }
			objectID_t ObjectID2() { return objectID2; }
			priority_t Priority() { return priority; }
			accessoryState_t AccessoryState() { return accessoryState; }
			lockState_t LockState() { return lockState; }

		protected:
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);

		private:
			objectType_t objectType1;
			objectID_t objectID1;
			objectType_t objectType2;
			objectID_t objectID2;
			priority_t priority;
			accessoryState_t accessoryState;
			lockState_t lockState;
	};
} // namespace datamodel

