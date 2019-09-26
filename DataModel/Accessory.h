#pragma once

#include <string>

#include "datatypes.h"
#include "DataModel/HardwareHandle.h"
#include "DataModel/LayoutItem.h"
#include "DataModel/LockableItem.h"

class Manager;

namespace DataModel
{
	class Accessory : public LayoutItem, public LockableItem, public HardwareHandle
	{
		public:
			enum accessoryType : accessoryType_t
			{
				AccessoryTypeDefault = 0
			};

			enum accessoryState : accessoryState_t
			{
				AccessoryStateOff = false,
				AccessoryStateOn = true
			};

			Accessory(__attribute__((unused)) Manager* manager, const accessoryID_t accessoryID)
			:	LayoutItem(accessoryID),
				lastUsed(0),
				counter(0)
			{
			}

			Accessory(const std::string& serialized)
			{
				Deserialize(serialized);
			}

			Accessory() {}

			virtual objectType_t GetObjectType() const { return ObjectTypeAccessory; }

			virtual std::string Serialize() const override;
			virtual bool Deserialize(const std::string& serialized) override;
			virtual std::string LayoutType() const override { return "accessory"; }

			void SetType(accessoryType_t type) { this->type = type; }
			accessoryType_t GetType() const { return type; }
			void SetState(accessoryState_t state) { this->state = state; lastUsed = time(nullptr); ++counter; }
			accessoryState_t GetState() const { return state; }
			void SetDuration(accessoryDuration_t duration) { this->duration = duration; }
			accessoryType_t GetDuration() const { return duration; }

			void SetInverted(const bool inverted) { this->inverted = inverted; }
			bool GetInverted() const { return inverted; }

			time_t GetLastUsed() const { return lastUsed; }

			static void Status(const accessoryState_t state, std::string& onText)
			{
				onText.assign(state == DataModel::Accessory::AccessoryStateOn ? "green" : "red");
			}


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
} // namespace DataModel

