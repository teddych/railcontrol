#pragma once

#include "datamodel/serializable.h"
#include "datatypes.h"
#include "manager.h"

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

			std::string Serialize() const override
			{
				std::string out(std::to_string(slaveID));
				out.append(speedRelation == SpeedRelationRailControl ? "R" : "D");
				return out;
			}

			bool Deserialize(const std::string& serialized) override
			{
				return true;
			}

			bool LoadAndCheckLocos();

			locoSpeed_t CalculateSlaveSpeed();

		private:
			void GetMasterLoco() { masterLoco = manager->GetLoco(masterID); }
			void GetSlaveLoco() { slaveLoco = manager->GetLoco(slaveID); }

			Manager* manager;
			locoID_t masterID;
			Loco* masterLoco;
			locoID_t slaveID;
			Loco* slaveLoco;
			speedRelation_t speedRelation;
	};
} // namespace datamodel
