#pragma once

#include "datatypes.h"

class Manager;

namespace hardware
{

	class HardwareParams
	{
		public:
			HardwareParams(controlID_t controlID, hardwareType_t hardwareType, std::string name, std::string arg1)
			:	controlID(controlID),
				hardwareType(hardwareType),
				name(name),
				arg1(arg1)
			{
			}

			HardwareParams(Manager* manager, controlID_t controlID, hardwareType_t hardwareType, std::string name, std::string arg1)
			:	manager(manager),
				controlID(controlID),
				hardwareType(hardwareType),
				name(name),
				arg1(arg1)
			{
			}

			Manager* manager;
			controlID_t controlID;
			hardwareType_t hardwareType;
			std::string name;
			std::string arg1;
	};

} // namespace hardware

