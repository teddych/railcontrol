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
	class Relation;

	class Street : public LayoutItem, public LockableItem
	{
		public:
			static const delay_t DefaultDelay = 250;

			enum commuterType_t : unsigned char
			{
				CommuterTypeNo = 0,
				CommuterTypeOnly = 1,
				CommuterTypeBoth = 2
			};

			Street(Manager* manager, const streetID_t streetID)
			:	LayoutItem(streetID),
			 	LockableItem(),
			 	manager(manager),
			 	fromTrack(TrackNone),
				lastUsed(0),
				counter(0)
			{
			}

			Street(Manager* manager, const std::string& serialized);

			~Street() { DeleteRelations(); }

			objectType_t GetObjectType() const { return ObjectTypeStreet; }

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "street"; };

			void DeleteRelations();
			bool AssignRelations(const std::vector<datamodel::Relation*>& newRelations);
			const std::vector<datamodel::Relation*>& GetRelations() const { return relations; };

			bool FromTrackDirection(const trackID_t trackID, const direction_t trackDirection, const datamodel::Loco* loco, const bool allowLocoTurn);

			bool Execute();
			static bool ExecuteStatic(Street* street) { return street->Execute(); }

			bool Reserve(const locoID_t locoID) override;
			bool Lock(const locoID_t locoID) override;
			bool Release(const locoID_t locoID) override;

			delay_t GetDelay() const { return delay; }
			void SetDelay(delay_t delay) { this->delay = delay; }
			commuterType_t GetCommuter() const { return commuter; }
			void SetCommuter(const commuterType_t commuter) { this->commuter = commuter; }
			length_t GetMinTrainLength() const { return minTrainLength; }
			void SetMinTrainLength(const length_t length) { this->minTrainLength = length; }
			length_t GetMaxTrainLength() const { return maxTrainLength; }
			void SetMaxTrainLength(const length_t length) { this->maxTrainLength = length; }
			time_t GetLastUsed() const { return lastUsed; }
			void SetAutomode(const automode_t automode) { this->automode = automode; }
			automode_t GetAutomode() const { return automode; }
			void SetFromTrack(const trackID_t fromTrack) { this->fromTrack = fromTrack; }
			trackID_t GetFromTrack() const { return fromTrack; }
			void SetFromDirection(const direction_t fromDirection) { this->fromDirection = fromDirection; }
			direction_t GetFromDirection() const { return fromDirection; }
			void SetToTrack(const trackID_t toTrack) { this->toTrack = toTrack; }
			trackID_t GetToTrack() const { return toTrack; };
			void SetToDirection(const direction_t toDirection) { this->toDirection = toDirection; }
			direction_t GetToDirection() const { return toDirection; }
			void SetFeedbackIdReduced(const feedbackID_t feedbackIdReduced) { this->feedbackIdReduced = feedbackIdReduced; }
			feedbackID_t GetFeedbackIdReduced() const { return feedbackIdReduced; }
			void SetFeedbackIdCreep(const feedbackID_t feedbackIdCreep) { this->feedbackIdCreep = feedbackIdCreep; }
			feedbackID_t GetFeedbackIdCreep() const { return feedbackIdCreep; }
			void SetFeedbackIdStop(const feedbackID_t feedbackIdStop) { this->feedbackIdStop = feedbackIdStop; }
			feedbackID_t GetFeedbackIdStop() const { return feedbackIdStop; }
			void SetFeedbackIdOver(const feedbackID_t feedbackIdOver) { this->feedbackIdOver = feedbackIdOver; }
			feedbackID_t GetFeedbackIdOver() const { return feedbackIdOver; }
			void SetWaitAfterRelease(const wait_t wait) { this->waitAfterRelease = wait; }
			wait_t GetWaitAfterRelease() const { return waitAfterRelease; }

			static bool CompareShortest(const Street* s1, const Street* s2) { return s1->GetMinTrainLength() < s2->GetMinTrainLength(); }
			static bool CompareLastUsed(const Street* s1, const Street* s2) { return s1->GetLastUsed() < s2->GetLastUsed(); }

		private:
			bool ReleaseInternal(const locoID_t locoID);
			void ReleaseInternalWithToTrack(const locoID_t locoID);

			Manager* manager;
			std::mutex updateMutex;

			delay_t delay;
			std::vector<datamodel::Relation*> relations;
			commuterType_t commuter;
			length_t minTrainLength;
			length_t maxTrainLength;
			automode_t automode;
			trackID_t fromTrack;
			direction_t fromDirection;
			trackID_t toTrack;
			direction_t toDirection;
			feedbackID_t feedbackIdReduced;
			feedbackID_t feedbackIdCreep;
			feedbackID_t feedbackIdStop;
			feedbackID_t feedbackIdOver;
			wait_t waitAfterRelease;

			time_t lastUsed;
			unsigned int counter;
	};
} // namespace datamodel

