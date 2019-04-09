#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "accessory.h"
#include "datatypes.h"
#include "serializable.h"

namespace datamodel
{
	class Switch : public Accessory
	{
		public:
			Switch(const switchID_t switchID,
				const std::string& name,
				const layoutPosition_t x,
				const layoutPosition_t y,
				const layoutPosition_t z,
				const layoutRotation_t rotation,
				const controlID_t controlID,
				const protocol_t protocol,
				const address_t address,
				const switchType_t type,
				const switchDuration_t timeout,
				const bool inverted)
			:	Accessory(switchID, name, x, y, z, rotation, controlID, protocol, address, type, timeout, inverted)
			{
			}

			Switch(const std::string& serialized)
			{
				Deserialize(serialized);
			}

			objectType_t GetObjectType() const { return ObjectTypeSwitch; }

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "switch"; };

			switchState_t GetState() const { return static_cast<switchState_t>(state); }
			switchType_t GetType() const { return static_cast<switchType_t>(type); }
	};

} // namespace datamodel

