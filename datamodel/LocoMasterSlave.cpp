#include "datamodel/Loco.h"
#include "datamodel/LocoMasterSlave.h"
#include "manager.h"

namespace datamodel
{
	std::string LocoMasterSlave::Serialize() const
	{
		std::string out(std::to_string(slaveID));
		out.append("-");
		out.append(speedRelation == SpeedRelationRailControl ? "R" : "D");
		return out;
	}

	bool LocoMasterSlave::Deserialize(const std::string& serialized)
	{
		std::vector<std::string> parts;
		Utils::Utils::SplitString(serialized, "-", parts);
		switch (parts.size())
		{
			case 2:
				slaveID = Utils::Utils::StringToInteger(parts[0]);
				speedRelation = parts[1].compare("R") == 0 ? SpeedRelationRailControl : SpeedRelationDecoder;
				return true;

			case 1:
				slaveID = Utils::Utils::StringToInteger(parts[0]);
				speedRelation = SpeedRelationDecoder;
				return true;

			default:
				slaveID = LocoNone;
				speedRelation = SpeedRelationDecoder;
				return false;
		}
	}

	bool LocoMasterSlave::LoadAndCheckLocos() const
	{
		if (masterLoco == nullptr)
		{
			LoadMasterLoco();
		}
		if (masterLoco == nullptr)
		{
			return false;
		}
		if (slaveLoco == nullptr)
		{
			LoadSlaveLoco();
		}
		return slaveLoco != nullptr;
	}

	locoSpeed_t LocoMasterSlave::CalculateSlaveSpeed() const
	{
		// we calculate with unsigned int, because unsigned short of locoSpeed_t is too small
		if (!LoadAndCheckLocos())
		{
			return MinSpeed;
		}

		locoSpeed_t masterSpeed = masterLoco->Speed();
		if (speedRelation == SpeedRelationDecoder)
		{
			return masterSpeed;
		}

		locoSpeed_t masterMax = masterLoco->GetMaxSpeed();
		locoSpeed_t slaveMax = slaveLoco->GetMaxSpeed();
		if (masterSpeed >= masterMax)
		{
			return slaveMax;
		}

		locoSpeed_t masterTravel = masterLoco->GetTravelSpeed();
		locoSpeed_t slaveTravel = slaveLoco->GetTravelSpeed();
		if (masterSpeed >= masterTravel)
		{
			unsigned int masterDiff = masterMax - masterTravel;
			unsigned int masterActual = masterSpeed - masterTravel;
			unsigned int slaveDiff = slaveMax - slaveTravel;
			unsigned int slaveActual = masterActual * slaveDiff / masterDiff;
			return slaveActual + slaveTravel;
		}

		locoSpeed_t masterReduced = masterLoco->GetReducedSpeed();
		locoSpeed_t slaveReduced = slaveLoco->GetReducedSpeed();
		if (masterSpeed >= masterReduced)
		{
			unsigned int masterDiff = masterTravel - masterReduced;
			unsigned int masterActual = masterSpeed - masterReduced;
			unsigned int slaveDiff = slaveTravel - slaveReduced;
			unsigned int slaveActual = masterActual * slaveDiff / masterDiff;
			return slaveActual + slaveReduced;
		}

		locoSpeed_t masterCreep = masterLoco->GetCreepSpeed();
		locoSpeed_t slaveCreep = slaveLoco->GetCreepSpeed();
		if (masterSpeed >= masterCreep)
		{
			unsigned int masterDiff = masterReduced - masterCreep;
			unsigned int masterActual = masterSpeed - masterCreep;
			unsigned int slaveDiff = slaveReduced - slaveCreep;
			unsigned int slaveActual = masterActual * slaveDiff / masterDiff;
			return slaveActual + slaveCreep;
		}

		unsigned int masterDiff = masterCreep;
		unsigned int masterActual = masterSpeed;
		unsigned int slaveDiff = slaveCreep;
		unsigned int slaveActual = masterActual * slaveDiff / masterDiff;
		return slaveActual;
	}

	void LocoMasterSlave::LoadMasterLoco() const
	{
		masterLoco = manager->GetLoco(masterID);
	}

	void LocoMasterSlave::LoadSlaveLoco() const
	{
		slaveLoco = manager->GetLoco(slaveID);
	}

	void LocoMasterSlave::SetSpeed()
	{
		manager->LocoSpeed(ControlTypeInternal, slaveLoco, CalculateSlaveSpeed());
	}
} // namespace datamodel
