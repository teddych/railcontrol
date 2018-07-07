#pragma once

#include <mutex>
#include <string>

#include "datatypes.h"
#include "object.h"
//#include "util/relation_vector.h"

class Manager;

namespace datamodel {

	class Street : public Object {
		public:
			Street(Manager* manager, const streetID_t streetID, const std::string& name, const blockID_t fromBlock, const direction_t fromDirection, const blockID_t toBlock, const direction_t toDirection, const feedbackID_t feedbackIDStop);
			Street(Manager* manager, const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			bool fromBlockDirection(blockID_t blockID, direction_t direction) { return (fromBlock == blockID && fromDirection == direction); }

			bool reserve(const locoID_t locoID);
			bool lock(const locoID_t locoID);
			bool release(const locoID_t locoID);
			locoID_t getLoco() const { return locoID; }
			lockState_t getState() const { return lockState; }

			blockID_t destinationBlock() const { return toBlock; };

			// FIXME: make private
			blockID_t fromBlock;
			direction_t fromDirection;
			blockID_t toBlock;
			direction_t toDirection;
			feedbackID_t feedbackIDStop;

		private:
			Manager* manager;
			lockState_t lockState;
			locoID_t locoID;
			std::mutex updateMutex;
	};

} // namespace datamodel

