/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2020 Dominik (Teddy) Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

#pragma once

#include <mutex>
#include <string>
#include <vector>

#include "DataModel/LayoutItem.h"
#include "DataModel/LockableItem.h"
#include "DataModel/TrackBase.h"
#include "DataTypes.h"
#include "Logger/Logger.h"

class Manager;

namespace DataModel
{
	class Loco;
	class Street;

	class Track : public TrackBase, public LayoutItem, public LockableItem
	{
		public:
			enum TrackType : unsigned char
			{
				TrackTypeStraight = 0,
				TrackTypeTurn = 1,
				TrackTypeEnd = 2,
				TrackTypeBridge = 3,
				TrackTypeTunnel = 4,
				TrackTypeTunnelEnd = 5,
				TrackTypeLink = 6
			};

			Track(Manager* manager, const TrackID trackID)
			:	TrackBase(manager),
				LayoutItem(trackID),
			 	LockableItem(),
			 	trackType(TrackTypeStraight)
			{
			}

			Track(Manager* manager, const std::string& serialized)
			:	TrackBase(manager),
				LayoutItem(TrackNone),
			 	LockableItem()
			{
				Deserialize(serialized);
			}

			ObjectType GetObjectType() const { return ObjectTypeTrack; }

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;

			std::string GetLayoutType() const override { return Languages::GetText(Languages::TextTrack); };
			TrackType GetTrackType() const { return trackType; }
			void SetTrackType(const TrackType type) { this->trackType = type; }

			bool Reserve(Logger::Logger* logger, const LocoID locoID) override
			{
				return BaseReserve(logger, locoID);
			}

			bool ReserveForce(Logger::Logger* logger, const LocoID locoID)
			{
				return BaseReserveForce(logger, locoID);
			}

			bool Lock(Logger::Logger* logger, const LocoID locoID) override
			{
				return BaseLock(logger, locoID);
			}

			bool Release(Logger::Logger* logger, const LocoID locoID) override
			{
				return BaseRelease(logger, locoID);
			}

			bool ReleaseForce(Logger::Logger* logger, const LocoID locoID)
			{
				return BaseReleaseForce(logger, locoID);
			}

		protected:
			bool ReserveInternal(Logger::Logger* logger, const LocoID locoID) override
			{
				return LockableItem::Reserve(logger, locoID);
			}

			bool LockInternal(Logger::Logger* logger, const LocoID locoID) override
			{
				return LockableItem::Lock(logger, locoID);
			}

			bool ReleaseInternal(Logger::Logger* logger, const LocoID locoID) override
			{
				return LockableItem::Release(logger, locoID);
			}

			void PublishTrackState() const override;

			ObjectID GetMyID() const override
			{
				return GetID();
			}

			const std::string& GetMyName() const override
			{
				return GetName();
			}

			LocoID GetLockedLoco() const override
			{
				return GetLoco();
			}

		private:
			TrackType trackType;
	};
} // namespace DataModel
