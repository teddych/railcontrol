#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "datatypes.h"
#include "datamodel/layout_item.h"
#include "datamodel/LockableItem.h"

class Manager;

namespace datamodel
{
	class Loco;
	class Street;

	class Track : public LayoutItem, public LockableItem
	{
		public:
			enum selectStreetApproach_t : unsigned char
			{
				SelectStreetSystemDefault = 0,
				SelectStreetDoNotCare = 1,
				SelectStreetRandom = 2,
				SelectStreetMinTrackLength = 3,
				SelectStreetLongestUnused = 4
			};

			Track(Manager* manager,
				const trackID_t trackID,
				const std::string& name,
				const layoutPosition_t x,
				const layoutPosition_t y,
				const layoutPosition_t z,
				const layoutItemSize_t height,
				const layoutRotation_t rotation,
				const trackType_t type,
				const std::vector<feedbackID_t>& feedbacks,
				const selectStreetApproach_t selectStreetApproach)
			:	LayoutItem(trackID, name, VisibleYes, x, y, z, Width1, height, rotation),
			 	LockableItem(),
			 	manager(manager),
				type(type),
				feedbacks(feedbacks),
				selectStreetApproach(selectStreetApproach),
				state(FeedbackStateFree),
			 	locoDirection(DirectionRight)
			{
			}

			Track(Manager* manager, const std::string& serialized)
			:	manager(manager)
			{
				Deserialize(serialized);
			}

			objectType_t GetObjectType() const { return ObjectTypeTrack; }

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;

			bool Reserve(const locoID_t locoID) override;
			bool ReserveForce(const locoID_t locoID) { return LockableItem::Reserve(locoID); }

			std::string LayoutType() const override { return "track"; };
			trackType_t GetType() const { return type; }
			void SetType(const trackType_t type) { this->type = type; }
			std::vector<feedbackID_t> GetFeedbacks() const { return feedbacks; }
			void Feedbacks(const std::vector<feedbackID_t>& feedbacks) { this->feedbacks = feedbacks; }

			bool FeedbackState(const feedbackID_t feedbackID, const feedbackState_t state);
			feedbackState_t FeedbackState() const { return state; };

			bool AddStreet(Street* street);
			bool RemoveStreet(Street* street);

			selectStreetApproach_t GetSelectStreetApproach() const { return selectStreetApproach; }
			void SetSelectStreetApproach(const selectStreetApproach_t selectStreetApproach) { this->selectStreetApproach = selectStreetApproach; }

			bool GetValidStreets(const datamodel::Loco* loco, std::vector<Street*>& validStreets) const;
			direction_t GetLocoDirection() const { return locoDirection; }
			void SetLocoDirection(const direction_t direction) { locoDirection = direction; }

		private:
			bool FeedbackStateInternal(const feedbackID_t feedbackID, const feedbackState_t state);
			void OrderValidStreets(std::vector<datamodel::Street*>& validStreets) const;
			selectStreetApproach_t GetSelectStreetApproachCalculated() const;

			Manager* manager;
			trackType_t type;
			std::vector<feedbackID_t> feedbacks;
			selectStreetApproach_t selectStreetApproach;
			feedbackState_t state;
			direction_t locoDirection;
			std::vector<Street*> streets;
			mutable std::mutex updateMutex;
	};
} // namespace datamodel
