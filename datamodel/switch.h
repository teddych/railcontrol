#pragma once

#include <string>

#include "accessory.h"
#include "datatypes.h"
#include "serializable.h"

namespace datamodel {

	class Switch : public Accessory {
		public:
			Switch(switchID_t switchID,
				std::string name,
				layoutPosition_t x,
				layoutPosition_t y,
				layoutPosition_t z,
				layoutRotation_t rotation,
				controlID_t controlID,
				protocol_t protocol,
				address_t address,
				switchType_t type,
				switchState_t state,
				switchTimeout_t timeout);

			Switch(const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;
			virtual std::string layoutType() const { return "switch"; };

		private:
			//lockState_t lockState;
	};

} // namespace datamodel

