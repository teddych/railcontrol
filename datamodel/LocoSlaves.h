#pragma once

#include <string>
#include <vector>

#include "datatypes.h"
#include "datamodel/LocoMasterSlave.h"
#include "datamodel/serializable.h"

class Manager;

namespace datamodel
{
	class LocoSlaves : Serializable
	{
		public:
			LocoSlaves() = delete;
			LocoSlaves(Manager* manager)
			:	manager(manager),
			 	masterID(LocoNone)
			{
			}
			LocoSlaves(Manager* manager, const locoID_t masterID)
			:	manager(manager),
			 	masterID(masterID)
			{
			}

			void SetMasterID(locoID_t locoID) { masterID = locoID; }
			std::string Serialize() const override;
			bool Deserialize(const std::string& serialized) override;

			void Set(const locoID_t slaveID, const LocoMasterSlave::speedRelation_t speedRelation);
			void Delete(const locoID_t slaveID);
			void SetSpeeds();
			std::vector<LocoMasterSlave> GetAll() const { return slaves; }

		private:
			void CheckAndSolveLoops();

			Manager* manager;
			locoID_t masterID;
			std::vector<LocoMasterSlave> slaves;
	};
} // namespace datamodel
