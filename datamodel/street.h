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
			Street(Manager* manager,
				const streetID_t streetID,
				const std::string& name,
				const delay_t delay,
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

			bool FromTrackDirection(trackID_t trackID, direction_t direction) { return fromTrack == trackID; }// FIXME: && fromDirection == direction; }

			bool Execute();
			static bool ExecuteStatic(Street* street) { return street->Execute(); }

			bool Reserve(const locoID_t locoID) override;
			bool Lock(const locoID_t locoID) override;
			bool Release(const locoID_t locoID) override;

			delay_t Delay() const { return delay; }
			void Delay(delay_t delay) { this->delay = delay; }

			trackID_t DestinationTrack() const { return toTrack; };

		private:
			bool ReleaseInternal(const locoID_t locoID);

			delay_t delay;

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

