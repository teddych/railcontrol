#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "datamodel/accessory.h"
#include "datatypes.h"

namespace datamodel
{
	class Signal : public Accessory
	{
		public:
			Signal(Manager* manager, const signalID_t signalID)
			:	Accessory(manager, signalID)
			{
			}

			Signal(const std::string& serialized)
			{
				Deserialize(serialized);
			}

			objectType_t GetObjectType() const { return ObjectTypeSignal; }

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "signal"; };

			signalState_t GetState() const { return static_cast<signalState_t>(state); }
			signalType_t GetType() const { return static_cast<signalType_t>(type); }
	};
} // namespace datamodel

