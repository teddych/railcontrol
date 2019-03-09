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
			Track(const trackID_t trackID,
				const std::string& name,
				const layoutPosition_t x,
				const layoutPosition_t y,
				const layoutPosition_t z,
				const layoutItemSize_t height,
				const layoutRotation_t rotation,
				const trackType_t type)
			:	LayoutItem(trackID, name, VisibleYes, x, y, z, Width1, height, rotation),
				type(type),
				lockState(LockStateFree),
			 	locoID(LocoNone),
			 	locoDirection(DirectionLeft)
			{
			}

			Track(const std::string& serialized)
			{
				Deserialize(serialized);
			}

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;
			std::string LayoutType() const override { return "track"; };
			trackType_t GetType() const { return type; }
			void Type(trackType_t type) { this->type = type; }

			bool Reserve(const locoID_t locoID);
			bool Lock(const locoID_t locoID);
			bool Release(const locoID_t locoID);
			locoID_t GetLoco() const { return locoID; }
			lockState_t GetState() const { return lockState; }

			bool AddStreet(Street* street);
			bool RemoveStreet(Street* street);

			bool ValidStreets(std::vector<Street*>& validStreets);

			bool IsInUse() const
			{
				return this->lockState != LockStateFree || this->locoID != LocoNone || this->streets.size() > 0;
			}

		private:
			trackType_t type;
			lockState_t lockState;
			locoID_t locoID;
			direction_t locoDirection;
			std::mutex updateMutex;
			std::vector<Street*> streets;
	};
} // namespace datamodel

