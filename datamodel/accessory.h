#pragma once

#include <string>

#include "datatypes.h"
#include "layout_item.h"
#include "LockableItem.h"
#include "serializable.h"

namespace datamodel
{
	class Accessory : public LayoutItem, public LockableItem
	{
		public:
			Accessory(const accessoryID_t accessoryID,
				const std::string& name,
				const layoutPosition_t x,
				const layoutPosition_t y,
				const layoutPosition_t z,
				const layoutRotation_t rotation,
				const controlID_t controlID,
				const protocol_t protocol,
				const address_t address,
				const accessoryType_t type,
				const accessoryDuration_t duration,
				const bool inverted)
			:	LayoutItem(accessoryID, name, VisibleYes, x, y, z, Width1, Height1, rotation),
				controlID(controlID),
				protocol(protocol),
				address(address),
				type(type),
				state(AccessoryStateOff),
				duration(duration),
				inverted(inverted),
				lastUsed(0),
				counter(0)
			{
			}

			Accessory(const std::string& serialized)
			{
				Deserialize(serialized);
			}

			Accessory() {}

			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;
			virtual std::string LayoutType() const override { return "accessory"; }

			void SetType(accessoryType_t type) { this->state = type; }
			accessoryType_t GetType() const { return type; }
			void SetState(accessoryState_t state) { this->state = state; lastUsed = time(nullptr); ++counter; }
			accessoryState_t GetState() const { return state; }
			void SetDuration(accessoryDuration_t duration) { this->duration = duration; }
			accessoryType_t GetDuration() const { return duration; }

			void SetInverted(const bool inverted) { this->inverted = inverted; }
			bool GetInverted() const { return inverted; }

			time_t GetLastUsed() const { return lastUsed; }

			// FIXME: make this private
			controlID_t controlID;
			protocol_t protocol;
			address_t address;

		protected:
			std::string SerializeWithoutType() const;
			virtual bool Deserialize(const std::map<std::string,std::string>& arguments);

			accessoryType_t type;
			accessoryState_t state;
			accessoryDuration_t duration; // duration in ms after which the accessory command will be turned off on rails. 0 = no turn off / turn off must be made manually
			bool inverted;

			time_t lastUsed;
			unsigned int counter;
	};
} // namespace datamodel

