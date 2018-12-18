#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "manager.h"
#include "serializable.h"

namespace datamodel {

	class Relation : protected Serializable {
		public:
			Relation(Manager* const manager,
				const relationID_t relationID,
				const std::string& name,
				const objectType_t objectType1,
				const objectID_t objectID1,
				const objectType_t objectType2,
				const objectID_t objectID2,
				const switchState_t switchState,
				const lockState_t lockState)
			:	manager(manager),
				relationID(relationID),
				name(name),
				objectType1(objectType1),
				objectID1(objectID1),
				objectType2(objectType2),
				objectID2(objectID2),
				switchState(switchState),
				lockState(lockState)
			{}

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;
			std::string& getName() { return name; }
			virtual bool execute(const locoID_t locoID);
			virtual bool release(const locoID_t locoID);

		protected:
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);

		private:
			Manager* const manager;
			relationID_t relationID;
			std::string name;
			objectType_t objectType1;
			objectID_t objectID1;
			objectType_t objectType2;
			objectID_t objectID2;
			switchState_t switchState;
			lockState_t lockState;
	};

} // namespace datamodel

