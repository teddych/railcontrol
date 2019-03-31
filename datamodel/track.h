#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "datatypes.h"
#include "layout_item.h"
#include "LockableItem.h"
#include "serializable.h"
#include "street.h"

namespace datamodel
{
	class Track : public LayoutItem, public LockableItem
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
			 	LockableItem(),
			 	manager(manager),
				type(type),
				feedbacks(feedbacks),
				state(FeedbackStateFree),
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

			bool Reserve(const locoID_t locoID) override;
			bool ReserveForce(const locoID_t locoID) { return LockableItem::Reserve(locoID); }

			std::string LayoutType() const override { return "track"; };
			trackType_t GetType() const { return type; }
			void Type(const trackType_t type) { this->type = type; }
			std::vector<feedbackID_t> GetFeedbacks() const { return feedbacks; }
			void Feedbacks(const std::vector<feedbackID_t>& feedbacks) { this->feedbacks = feedbacks; }

			bool FeedbackState(const feedbackID_t feedbackID, const feedbackState_t state);
			feedbackState_t FeedbackState() const { return state; };

			bool AddStreet(Street* street);
			bool RemoveStreet(Street* street);

			bool ValidStreets(std::vector<Street*>& validStreets);

		private:
			bool FeedbackStateInternal(const feedbackID_t feedbackID, const feedbackState_t state);

			Manager* manager;
			trackType_t type;
			std::vector<feedbackID_t> feedbacks;
			feedbackState_t state;
			direction_t locoDirection;
			std::vector<Street*> streets;
			std::mutex updateMutex;
	};
} // namespace datamodel

