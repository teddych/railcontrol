#include "datamodel/LocoSlaves.h"
#include "manager.h"

using std::map;
using std::stringstream;
using std::string;
using std::vector;

namespace datamodel
{
	std::string LocoSlaves::Serialize() const
	{
		stringstream ss;
		for (size_t i = 0; i < slaves.size(); ++i)
		{
			if (i > 0)
			{
				ss << ",";
			}
			ss << slaves[i].Serialize();
		}
		return ss.str();
	}

	bool LocoSlaves::Deserialize(const std::string& serialized)
	{
		std::vector<std::string> slaveParts;
		Utils::Utils::SplitString(serialized, ",", slaveParts);
		for (size_t i = 0; i < slaveParts.size(); ++i)
		{
			LocoMasterSlave slave(manager, masterID, slaveParts[i]);
			slaves.push_back(slave);
		}
		return true;
	}

	void LocoSlaves::Set(const locoID_t slaveID, const LocoMasterSlave::speedRelation_t speedRelation)
	{
		Delete(slaveID);
		slaves.push_back(LocoMasterSlave(manager, masterID, slaveID, speedRelation));
		CheckAndSolveLoops();
	}

	void LocoSlaves::CheckAndSolveLoops()
	{
		for (size_t index = 0; index < slaves.size(); ++index)
		{
			Loco* slave = slaves[index].GetSlave();
			slave->DeleteSlave(masterID);
		}
	}

	void LocoSlaves::Delete(const locoID_t slaveID)
	{
		for (size_t index = 0; index < slaves.size(); ++index)
		{
			if (slaveID != slaves[index].GetSlaveID())
			{
				continue;
			}
			slaves.erase(slaves.begin() + index);
			return;
		}
	}

	void LocoSlaves::SetSpeeds()
	{
		for (size_t index = 0; index < slaves.size(); ++index)
		{
			slaves[index].SetSpeed();
		}
	}
} // namespace datamodel
