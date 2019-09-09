#pragma once

#include "datamodel/serializable.h"
#include "datatypes.h"
#include "Utils/Utils.h"

class Manager;

namespace datamodel
{
	class Loco;

	class LocoMasterSlave : Serializable
	{
		public:
			enum speedRelation_t : unsigned char
			{
				SpeedRelationDecoder,
				SpeedRelationRailControl
			};

			LocoMasterSlave() = delete;
			LocoMasterSlave(Manager* manager, locoID_t masterID, std::string& serialized)
			:	manager(manager),
			 	masterID(masterID),
			 	masterLoco(nullptr),
			 	slaveID(LocoNone),
			 	slaveLoco(nullptr),
			 	speedRelation(SpeedRelationDecoder)
			{
				Deserialize(serialized);
			}

			LocoMasterSlave(Manager* manager, locoID_t masterID, locoID_t slaveID, speedRelation_t speedRelation)
			:	manager(manager),
			 	masterID(masterID),
			 	masterLoco(nullptr),
			 	slaveID(slaveID),
			 	slaveLoco(nullptr),
			 	speedRelation(speedRelation)
			{}

			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;

			bool LoadAndCheckLocos() const;

			locoSpeed_t CalculateSlaveSpeed() const;
			locoID_t GetSlaveID() const { return slaveID; }
			Loco* GetSlave() const { return slaveLoco; }
			speedRelation_t GetSpeedRelation() const { return speedRelation; }
			void SetSpeed();

		private:
			void LoadMasterLoco() const;
			void LoadSlaveLoco() const;

			Manager* manager;
			locoID_t masterID;
			mutable Loco* masterLoco;
			locoID_t slaveID;
			mutable Loco* slaveLoco;
			speedRelation_t speedRelation;
	};
} // namespace datamodel
