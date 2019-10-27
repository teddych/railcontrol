/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2019 Dominik (Teddy) Mahrer - www.railcontrol.org

RailControl is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3, or (at your option) any
later version.

RailControl is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with RailControl; see the file LICENCE. If not see
<http://www.gnu.org/licenses/>.
*/

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

