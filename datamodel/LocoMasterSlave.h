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

			bool LoadAndCheckLocos();

			locoSpeed_t CalculateSlaveSpeed();

		private:
			void GetMasterLoco();
			void GetSlaveLoco();

			Manager* manager;
			locoID_t masterID;
			Loco* masterLoco;
			locoID_t slaveID;
			Loco* slaveLoco;
			speedRelation_t speedRelation;
	};
} // namespace datamodel
