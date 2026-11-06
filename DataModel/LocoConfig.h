/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2026 by Teddy / Dominik Mahrer - www.railcontrol.org

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

#include <string>

#include "DataModel/Loco.h"
#include "DataModel/MultipleUnit.h"
#include "DataModel/ObjectIdentifier.h"
#include "Hardware/LocoCache.h"

namespace DataModel
{
	class LocoConfig
	{
		public:
			inline LocoConfig(const LocoType type = LocoTypeNone)
			:	type(type),
				controlID(ControlNone),
				locoID(LocoNone),
				protocol(ProtocolNone),
				address(AddressNone),
				mfxUid(0),
				serverAddress(AddressNone),
				isInUse(false),
				image(),
				cs2image(),
				pushpull(false),
				length(0),
				maxSpeed(MaxSpeed),
				travelSpeed(DefaultTravelSpeed),
				reducedSpeed(DefaultReducedSpeed),
				creepingSpeed(DefaultCreepingSpeed),
				propulsion(PropulsionUnknown),
				trainType(TrainTypeUnknown),
				speed(MinSpeed),
				orientation(OrientationLeft)
			{
			}

			inline LocoConfig(const DataModel::Loco& loco)
			:	type(LocoTypeLoco),
				controlID(loco.GetControlID()),
				locoID(loco.GetID()),
				protocol(loco.GetProtocol()),
				address(loco.GetAddress()),
				mfxUid(loco.GetMfxUid()),
				serverAddress(loco.GetServerAddress()),
				name(loco.GetName()),
				matchKey(loco.GetMatchKey()),
				isInUse(loco.IsInUse()),
				image(loco.GetImage()),
				cs2image(loco.GetCs2Image()),
				pushpull(loco.GetPushpull()),
				length(loco.GetLength()),
				maxSpeed(loco.GetMaxSpeed()),
				travelSpeed(loco.GetTravelSpeed()),
				reducedSpeed(loco.GetReducedSpeed()),
				creepingSpeed(loco.GetCreepingSpeed()),
				propulsion(loco.GetPropulsion()),
				trainType(loco.GetTrainType()),
				speed(loco.GetSpeed()),
				orientation(loco.GetOrientation())
			{
				SetFunctionStates(loco.GetFunctionStates());
			}

			inline LocoConfig(const Hardware::LocoCacheEntry& loco)
			:	type(loco.GetType()),
				controlID(loco.GetControlID()),
				locoID(loco.GetLocoID()),
				protocol(loco.GetProtocol()),
				address(loco.GetAddress()),
				mfxUid(loco.GetMfxUid()),
				serverAddress(AddressNone),
				name(loco.GetName()),
				matchKey(loco.GetMatchKey()),
				isInUse(false),
				image(),
				cs2image(),
				pushpull(false),
				length(0),
				maxSpeed(MinSpeed),
				travelSpeed(MinSpeed),
				reducedSpeed(MinSpeed),
				creepingSpeed(MinSpeed),
				propulsion(PropulsionUnknown),
				trainType(TrainTypeUnknown),
				slaves(loco.GetSlaveIDs()),
				speed(MinSpeed),
				orientation(OrientationLeft)
			{
				SetFunctionStates(loco.GetFunctionStates());
			}

			inline LocoConfig(const DataModel::MultipleUnit& multipleUnit)
			:	type(LocoTypeMultipleUnit),
				controlID(multipleUnit.GetControlID()),
				locoID(multipleUnit.GetID()),
				protocol(multipleUnit.GetProtocol()),
				address(multipleUnit.GetAddress()),
				mfxUid(0),
				serverAddress(multipleUnit.GetServerAddress()),
				name(multipleUnit.GetName()),
				matchKey(multipleUnit.GetMatchKey()),
				isInUse(multipleUnit.IsInUse()),
				image(multipleUnit.GetImage()),
				cs2image(multipleUnit.GetCs2Image()),
				pushpull(multipleUnit.GetPushpull()),
				length(multipleUnit.GetLength()),
				maxSpeed(multipleUnit.GetMaxSpeed()),
				travelSpeed(multipleUnit.GetTravelSpeed()),
				reducedSpeed(multipleUnit.GetReducedSpeed()),
				creepingSpeed(multipleUnit.GetCreepingSpeed()),
				propulsion(multipleUnit.GetPropulsion()),
				trainType(multipleUnit.GetTrainType()),
				slaves(multipleUnit.GetSlaveIDs()),
				speed(multipleUnit.GetSpeed()),
				orientation(multipleUnit.GetOrientation())
			{
				SetFunctionStates(multipleUnit.GetFunctionStates());
			}

			inline LocoConfig(const DataModel::LocoBase& locoBase)
			:	type(locoBase.GetLocoType()),
				controlID(locoBase.GetControlID()),
				locoID(locoBase.GetID()),
				protocol(locoBase.GetProtocol()),
				address(locoBase.GetAddress()),
				mfxUid(0),
				serverAddress(locoBase.GetServerAddress()),
				name(locoBase.GetName()),
				matchKey(locoBase.GetMatchKey()),
				isInUse(locoBase.IsInUse()),
				image(locoBase.GetImage()),
				cs2image(locoBase.GetCs2Image()),
				pushpull(locoBase.GetPushpull()),
				length(locoBase.GetLength()),
				maxSpeed(locoBase.GetMaxSpeed()),
				travelSpeed(locoBase.GetTravelSpeed()),
				reducedSpeed(locoBase.GetReducedSpeed()),
				creepingSpeed(locoBase.GetCreepingSpeed()),
				propulsion(locoBase.GetPropulsion()),
				trainType(locoBase.GetTrainType()),
				speed(locoBase.GetSpeed()),
				orientation(locoBase.GetOrientation())
			{
				SetFunctionStates(locoBase.GetFunctionStates());
			}

			inline LocoType GetType() const
			{
				return type;
			}

			inline ControlID GetControlID() const
			{
				return controlID;
			}

			inline LocoID GetLocoID() const
			{
				return locoID;
			}

			inline DataModel::ObjectIdentifier GetObjectIdentifier() const
			{
				switch (type)
				{
					case LocoTypeLoco:
						return DataModel::ObjectIdentifier(ObjectTypeLoco, locoID);

					case LocoTypeMultipleUnit:
						return DataModel::ObjectIdentifier(ObjectTypeMultipleUnit, locoID);

					default:
						return DataModel::ObjectIdentifier(ObjectTypeNone, LocoNone);
				}
			}

			inline Protocol GetProtocol() const
			{
				return protocol;
			}

			inline Address GetAddress() const
			{
				return address;
			}

			inline uint32_t GetMfxUid() const
			{
				return mfxUid;
			}

			inline Address GetServerAddress() const
			{
				return serverAddress;
			}

			inline const std::string& GetName() const
			{
				return name;
			}

			inline const std::string& GetMatchKey() const
			{
				return matchKey;
			}

			inline bool IsInUse() const
			{
				return isInUse;
			}

			inline const std::string& GetImage() const
			{
				return image;
			}

			inline const std::string& GetCs2Image() const
			{
				return cs2image;
			}

			inline std::vector<DataModel::LocoFunctionEntry> GetFunctionStates() const
			{
				return functions.GetFunctionStates();
			}

			inline void GetFunctionStates(LocoFunctionEntry* out) const
			{
				functions.GetFunctions(out);
			}

			inline void SetFunctionStates(const std::vector<LocoFunctionEntry>& newEntries)
			{
				functions.SetFunctionStates(newEntries);
			}

			inline bool GetPushpull() const
			{
				return pushpull;
			}

			inline Length GetLength() const
			{
				return length;
			}

			inline Speed GetMaxSpeed() const
			{
				return maxSpeed;
			}

			inline Speed GetTravelSpeed() const
			{
				return travelSpeed;
			}

			inline Speed GetReducedSpeed() const
			{
				return reducedSpeed;
			}

			inline Speed GetCreepingSpeed() const
			{
				return creepingSpeed;
			}

			inline Propulsion GetPropulsion() const
			{
				return propulsion;
			}

			inline TrainType GetTrainType() const
			{
				return trainType;
			}

			inline const std::vector<LocoID>& GetSlaves() const
			{
				return slaves;
			}

			inline Speed GetSpeed() const
			{
				return speed;
			}

			inline Orientation GetOrientation() const
			{
				return orientation;
			}

			inline bool IsValid() const
			{
				return (locoID != LocoNone) && (type != LocoTypeNone);
			}

			inline void SetControlID(const ControlID controlID)
			{
				this->controlID = controlID;
			}

			inline void SetLocoID(const LocoID locoID)
			{
				this->locoID = locoID;
			}

			inline void SetProtocol(const Protocol protocol)
			{
				this->protocol = protocol;
			}

			inline void SetAddress(const Address address)
			{
				this->address = address;
			}

			inline void SetMfxUid(const MfxUid mfxUid)
			{
				this->mfxUid = mfxUid;
			}

			inline void SetServerAddress(const Address serverAddress)
			{
				this->serverAddress = serverAddress;
			}

			inline void SetName(const std::string& name)
			{
				this->name = name;
			}

			inline void SetMatchKey(const std::string& matchKey)
			{
				this->matchKey = matchKey;
			}

			inline void SetImage(const std::string& image)
			{
				this->image = image;
			}

			inline void SetCs2Image(const std::string& cs2image)
			{
				this->cs2image = cs2image;
			}

			inline void SetPushpull(const bool pushpull)
			{
				this->pushpull = pushpull;
			}

			inline void SetLength(const Length length)
			{
				this->length = length;
			}

			inline void SetMaxSpeed(const Speed maxSpeed)
			{
				this->maxSpeed = maxSpeed;
			}

			inline void SetTravelSpeed(const Speed travelSpeed)
			{
				this->travelSpeed = travelSpeed;
			}

			inline void SetReducedSpeed(const Speed reducedSpeed)
			{
				this->reducedSpeed = reducedSpeed;
			}

			inline void SetCreepingSpeed(const Speed creepingSpeed)
			{
				this->creepingSpeed = creepingSpeed;
			}

			inline void SetPropulsion(const Propulsion propulsion)
			{
				this->propulsion = propulsion;
			}

			inline void SetTrainType(const TrainType trainType)
			{
				this->trainType = trainType;
			}

		private:
			const LocoType type;
			ControlID controlID;
			LocoID locoID;
			Protocol protocol;
			Address address;
			uint32_t mfxUid;
			Address serverAddress;
			std::string name;
			std::string matchKey;
			const bool isInUse;
			std::string image;
			std::string cs2image;
			LocoFunctions functions;
			bool pushpull;
			Length length;
			Speed maxSpeed;
			Speed travelSpeed;
			Speed reducedSpeed;
			Speed creepingSpeed;
			Propulsion propulsion;
			TrainType trainType;
			const std::vector<LocoID> slaves;

			const Speed speed;
			const Orientation orientation;
	};
} // namespace DataModel
