#include "datamodel/Loco.h"
#include "datamodel/LocoMasterSlave.h"

namespace datamodel
{
	bool LocoMasterSlave::LoadAndCheckLocos()
	{
		if (masterLoco == nullptr)
		{
			GetMasterLoco();
		}
		if (masterLoco == nullptr)
		{
			return false;
		}
		if (slaveLoco == nullptr)
		{
			GetSlaveLoco();
		}
		return slaveLoco != nullptr;
	}

	locoSpeed_t LocoMasterSlave::CalculateSlaveSpeed()
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
} // namespace datamodel
