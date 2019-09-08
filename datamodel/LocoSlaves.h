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

		private:
			Manager* manager;
			locoID_t masterID;
			std::vector<LocoMasterSlave> slaves;
	};
} // namespace datamodel
