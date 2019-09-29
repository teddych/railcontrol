#pragma once

#include "DataTypes.h"

class Manager;

namespace Hardware
{

	class HardwareParams
	{
		public:
			HardwareParams(controlID_t controlID,
				hardwareType_t hardwareType,
				std::string name,
				std::string arg1,
				std::string arg2,
				std::string arg3,
				std::string arg4,
				std::string arg5)
			:	controlID(controlID),
				hardwareType(hardwareType),
				name(name),
				arg1(arg1),
				arg2(arg2),
				arg3(arg3),
				arg4(arg4),
				arg5(arg5)
			{
			}

			HardwareParams(Manager* manager,
				controlID_t controlID,
				hardwareType_t hardwareType,
				std::string name,
				std::string arg1,
				std::string arg2,
				std::string arg3,
				std::string arg4,
				std::string arg5)
			:	manager(manager),
				controlID(controlID),
				hardwareType(hardwareType),
				name(name),
				arg1(arg1),
				arg2(arg2),
				arg3(arg3),
				arg4(arg4),
				arg5(arg5)
			{
			}

			Manager* manager;
			controlID_t controlID;
			hardwareType_t hardwareType;
			std::string name;
			std::string arg1;
			std::string arg2;
			std::string arg3;
			std::string arg4;
			std::string arg5;
	};

} // namespace Hardware

