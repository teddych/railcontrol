#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "datatypes.h"
#include "layout_item.h"
#include "serializable.h"
#include "street.h"

namespace datamodel
{
	class Track : public LayoutItem
	{
		public:
			Track(Manager* manager,
				const trackID_t trackID,
				const std::string& name,
				const layoutPosition_t x,
				const layoutPosition_t y,
				const layoutPosition_t z,
				const layoutItemSize_t height,
				const layoutRotation_t rotation,
				const trackType_t type,
				const std::vector<feedbackID_t>& feedbacks)
			:	LayoutItem(trackID, name, VisibleYes, x, y, z, Width1, height, rotation),
			 	manager(manager),
				type(type),
				feedbacks(feedbacks),
				state(FeedbackStateFree),
				lockState(LockStateFree),
			 	locoID(LocoNone),
			 	locoDirection(DirectionLeft)
			{
			}

			Track(Manager* manager, const std::string& serialized)
			:	manager(manager)
			{
				Deserialize(serialized);
			}

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "track"; };
			trackType_t GetType() const { return type; }
			void Type(const trackType_t type) { this->type = type; }
			std::vector<feedbackID_t> GetFeedbacks() const { return feedbacks; }
			void Feedbacks(const std::vector<feedbackID_t>& feedbacks) { this->feedbacks = feedbacks; }

			bool FeedbackState(const feedbackID_t feedbackID, const feedbackState_t state);
			feedbackState_t FeedbackState() const { return state; };

			bool Reserve(const locoID_t locoID);
			bool Lock(const locoID_t locoID);
			bool Release(const locoID_t locoID);
			locoID_t GetLoco() const { return locoID; }
			lockState_t GetLockState() const { return lockState; }

			bool AddStreet(Street* street);
			bool RemoveStreet(Street* street);

			bool ValidStreets(std::vector<Street*>& validStreets);

			bool IsInUse() const
			{
				return this->lockState != LockStateFree || this->locoID != LocoNone;
			}

		private:
			Manager* manager;
			trackType_t type;
			std::vector<feedbackID_t> feedbacks;
			feedbackState_t state;
			lockState_t lockState;
			locoID_t locoID;
			direction_t locoDirection;
			std::mutex updateMutex;
			std::vector<Street*> streets;
	};
} // namespace datamodel

