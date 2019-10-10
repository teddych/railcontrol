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

#include <sstream>
#include <string>

#include "Hardware/Virtual.h"
#include "Manager.h"
#include "Utils/Utils.h"

namespace Hardware
{

	// create_virt and destroy_virt are used to instantiate
	// and delete the command station in main program

	// create instance of virtual
	extern "C" Virtual* create_Virtual(const HardwareParams* params)
	{
		return new Virtual(params);
	}

	// delete instance of virtual
	extern "C" void destroy_Virtual(Virtual* virt)
	{
		delete(virt);
	}


	Virtual::Virtual(const HardwareParams* params)
	:	HardwareInterface(params->manager, params->controlID, "Virtual Command Station / " + params->name),
	 	logger(Logger::Logger::GetLogger("Virtual " + params->name))
	{}

	// turn booster on or off
	void Virtual::Booster(const boosterState_t status)
	{
		logger->Info("Turning booster {0}", status ? "on" : "off");
	}

	// set loco speed
	void Virtual::LocoSpeed(const protocol_t& protocol, const address_t& address, const locoSpeed_t& speed)
	{
		logger->Info("Setting speed of loco {0}/{1} to speed {2}", protocol, address, speed);
	}

	// set the direction of a loco
	void Virtual::LocoDirection(const protocol_t& protocol, const address_t& address, const direction_t& direction)
	{
		logger->Info("Setting direction of loco {0}/{1} to {2}", protocol, address, direction == DirectionRight ? "forward" : "reverse");
	}

	// set loco function
	void Virtual::LocoFunction(const protocol_t protocol, const address_t address, const function_t function, const bool on)
	{
		logger->Info("Setting f%i of loco {0}/{1} to \"{2}\"", static_cast<int>(function), static_cast<int>(protocol), static_cast<int>(address), on ? "on" : "off");
	}

	// accessory command
	void Virtual::Accessory(const protocol_t protocol, const address_t address, const accessoryState_t state, const bool on)
	{
		std::string stateText;
		DataModel::Accessory::Status(state, stateText);
		logger->Info("Setting state of virtual accessory {0}/{1}/{2} to \"{3}\"", static_cast<int>(protocol), static_cast<int>(address), stateText, on ? "on" : "off");
	}

} // namespace
