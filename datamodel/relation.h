#pragma once

#include <map>
#include <string>

#include "datatypes.h"
#include "serializable.h"

namespace datamodel {

	class Relation : protected Serializable {
		public:
			Relation() {}
			Relation(const relationID_t relationID, const std::string& name, const objectType_t objectType1, const objectID_t objectID1, const objectType_t objectType2, const objectID_t objectID2, const switchState_t switchState, const lockState_t lockState);

			virtual std::string serialize() const override;
			virtual bool deserialize(const std::string& serialized) override;
			std::string& getName() { return name; }

		protected:
			virtual bool deserialize(const std::map<std::string,std::string>& arguments);

		private:
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

