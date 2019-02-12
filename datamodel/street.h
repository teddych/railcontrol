#pragma once

#include <mutex>
#include <string>

#include "datatypes.h"
#include "datamodel/layout_item.h"
#include "datamodel/relation.h"

class Manager;

namespace datamodel
{

	class Street : public LayoutItem
	{
		public:
			Street(Manager* manager,
				const streetID_t streetID,
				const std::string& name,
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
				const feedbackID_t feedbackIDStop);

			Street(Manager* manager, const std::string& serialized);

			~Street() { DeleteRelations(); }

			std::string serialize() const override;
			bool deserialize(const std::string& serialized) override;

			void DeleteRelations();
			bool AssignRelations(const std::vector<datamodel::Relation*>& newRelations);
			const std::vector<datamodel::Relation*>& GetRelations() const { return relations; };

			bool fromTrackDirection(trackID_t trackID, direction_t direction) { return (fromTrack == trackID && fromDirection == direction); }

			bool Execute();
			static bool ExecuteStatic(Street* street) { return street->Execute(); }

			bool reserve(const locoID_t locoID);
			bool lock(const locoID_t locoID);
			bool release(const locoID_t locoID);
			locoID_t getLoco() const { return locoID; }
			lockState_t getState() const { return lockState; }

			trackID_t destinationTrack() const { return toTrack; };

			// FIXME: make private
			automode_t automode;
			trackID_t fromTrack;
			direction_t fromDirection;
			trackID_t toTrack;
			direction_t toDirection;
			feedbackID_t feedbackIDStop;

		private:
			Manager* manager;
			std::vector<datamodel::Relation*> relations;
			lockState_t lockState;
			locoID_t locoID;
			std::mutex updateMutex;
	};

} // namespace datamodel

