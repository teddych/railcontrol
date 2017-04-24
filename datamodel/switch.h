#ifndef DATAMODEL_SWITCH_H
#define DATAMODEL_SWITCH_H

#include <string>

#include "accessory.h"
#include "datatypes.h"
#include "serializable.h"

namespace datamodel {

	class Switch : public Accessory {
		public:
			Switch(switchID_t switchID,
			       std::string name,
						 controlID_t controlID,
						 protocol_t protocol,
						 address_t address,
						 switchType_t type,
						 switchState_t state,
						 layoutRotation_t rotation,
						 layoutPosition_t x,
						 layoutPosition_t y,
						 layoutPosition_t z);

			Switch(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			static void getTexts(const switchState_t state, char*& stateText);

			switchID_t switchID;
	};

} // namespace datamodel

#endif // DATAMODEL_SWITCH_H
