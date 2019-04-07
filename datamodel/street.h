#pragma once

#include <mutex>
#include <string>

#include "datatypes.h"
#include "datamodel/layout_item.h"
#include "datamodel/relation.h"

class Manager;

namespace datamodel
{
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

			Street(Manager* manager,
				const streetID_t streetID,
				const std::string& name,
				const delay_t delay,
				const commuterType_t commuter,
				const length_t minTrainLength,
				const length_t maxTrainLength,
				const std::vector<datamodel::Relation*>& relations,
				const visible_t visible,
				const layoutPosition_t posX,
				const layoutPosition_t posY,
				const layoutPosition_t posZ,
				const automode_t automode,
				const trackID_t fromTrack,
				const direction_t fromDirection,
				const trackID_t toTrack,
				const direction_t toDirection,
				const feedbackID_t feedbackIdReduced,
				const feedbackID_t feedbackIdCreep,
				const feedbackID_t feedbackIdStop,
				const feedbackID_t feedbackIdOver);

			Street(Manager* manager, const std::string& serialized);

			~Street() { DeleteRelations(); }

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "street"; };

			void DeleteRelations();
			bool AssignRelations(const std::vector<datamodel::Relation*>& newRelations);
			const std::vector<datamodel::Relation*>& GetRelations() const { return relations; };

			bool FromTrackDirection(const trackID_t trackID, const direction_t trackDirection, const bool commuter);

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

			trackID_t DestinationTrack() const { return toTrack; };

			static bool CompareShortest(const Street* s1, const Street* s2) { return s1->GetMinTrainLength() < s2->GetMinTrainLength(); }

		private:
			bool ReleaseInternal(const locoID_t locoID);

			delay_t delay;
			commuterType_t commuter;
			length_t minTrainLength;
			length_t maxTrainLength;

		public:
			// FIXME: make private
			automode_t automode;
			trackID_t fromTrack;
			direction_t fromDirection;
			trackID_t toTrack;
			direction_t toDirection;
			feedbackID_t feedbackIdReduced;
			feedbackID_t feedbackIdCreep;
			feedbackID_t feedbackIdStop;
			feedbackID_t feedbackIdOver;

		private:
			Manager* manager;
			std::vector<datamodel::Relation*> relations;
			std::mutex updateMutex;
	};

} // namespace datamodel

