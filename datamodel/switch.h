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

			bool reserve(const locoID_t locoID);
			bool hardLock(const locoID_t locoID, const switchState_t switchState);
			bool softLock(const locoID_t locoID, const switchState_t switchState);
			bool release(const locoID_t locoID);

		private:
			lockState_t lockState;
			locoID_t locoIDHardLock;
			//vector<locoID_t> locoIDSoftLock;
			std::mutex updateMutex;
	};

} // namespace datamodel

