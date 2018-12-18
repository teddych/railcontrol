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
			Street(Manager* manager, const streetID_t streetID, const std::string& name, const trackID_t fromTrack, const direction_t fromDirection, const trackID_t toTrack, const direction_t toDirection, const feedbackID_t feedbackIDStop);
			Street(Manager* manager, const std::string& serialized);

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			bool fromTrackDirection(trackID_t trackID, direction_t direction) { return (fromTrack == trackID && fromDirection == direction); }

			bool reserve(const locoID_t locoID);
			bool lock(const locoID_t locoID);
			bool release(const locoID_t locoID);
			locoID_t getLoco() const { return locoID; }
			lockState_t getState() const { return lockState; }

			trackID_t destinationTrack() const { return toTrack; };

			// FIXME: make private
			trackID_t fromTrack;
			direction_t fromDirection;
			trackID_t toTrack;
			direction_t toDirection;
			feedbackID_t feedbackIDStop;

		private:
			Manager* manager;
			lockState_t lockState;
			locoID_t locoID;
			std::mutex updateMutex;
	};

} // namespace datamodel

