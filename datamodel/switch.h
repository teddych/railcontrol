#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "accessory.h"
#include "datatypes.h"
#include "serializable.h"

namespace datamodel {

	class Switch : public Accessory {
		public:
			// FIXME: const
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
				switchTimeout_t timeout,
				bool inverted)
			:	Accessory(switchID, name, x, y, z, rotation, controlID, protocol, address, type, timeout, inverted)
			{
			}

			Switch(const std::string& serialized)
			{
				Deserialize(serialized);
			}

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "switch"; };

			switchState_t GetState() const { return static_cast<switchState_t>(state); }
			switchType_t GetType() const { return static_cast<switchType_t>(type); }
	};

} // namespace datamodel

