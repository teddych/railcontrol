#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "datamodel/accessory.h"
#include "datatypes.h"

namespace datamodel
{
	class Switch : public Accessory
	{
		public:
			enum switchType : switchType_t
			{
				SwitchTypeLeft = 0,
				SwitchTypeRight
			};

			enum switchState : switchState_t
			{
				SwitchStateTurnout = false,
				SwitchStateStraight = true
			};

			Switch(Manager* manager, const switchID_t switchID)
			:	Accessory(manager, switchID)
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

