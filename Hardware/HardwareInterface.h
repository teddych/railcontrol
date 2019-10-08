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

#include <map>
#include <string>
#include <vector>

#include "DataTypes.h"
#include "Manager.h"

namespace Hardware
{

	class HardwareInterface
	{
		public:
			// non virtual default constructor is needed to prevent polymorphism
			HardwareInterface(Manager* manager, const controlID_t controlID, const std::string& name)
			:	manager(manager),
			 	controlID(controlID),
			 	name(name)
			{};

			// pure virtual destructor prevents polymorphism in derived class
			virtual ~HardwareInterface() {};

			// get the name of the hardware
			const std::string GetName() const { return name; }

			// can this control handle locos
			virtual bool CanHandleLocos() const { return false; }

			// can this control handle accessories, switches, ...
			virtual bool CanHandleAccessories() const { return false; }

			// can this control handle feedback
			virtual bool CanHandleFeedback() const { return false; }

			// get available loco protocols of this control
			virtual void GetLocoProtocols(std::vector<protocol_t>& protocols) const { protocols.push_back(ProtocolNone); }

			// is given loco protocol supported
			virtual bool LocoProtocolSupported(__attribute__((unused))  const protocol_t protocol) const { return false; }

			// get available accessory protocols of this control
			virtual void GetAccessoryProtocols(__attribute__((unused)) std::vector<protocol_t>& protocols) const { protocols.push_back(ProtocolNone); }

			// is given accessory protocol supported
			virtual bool AccessoryProtocolSupported(__attribute__((unused)) const protocol_t protocol) const { return false; }

			// get types of needed arguments of this control
			virtual void GetArgumentTypes(__attribute__((unused)) std::map<unsigned char,argumentType_t>& argumentTypes) const {}

			// turn booster on or off
			virtual void Booster(__attribute__((unused)) const boosterState_t status) {};

			// set loco speed
			virtual void LocoSpeed(__attribute__((unused)) const protocol_t& protocol, __attribute__((unused)) const address_t& address, __attribute__((unused)) const locoSpeed_t& speed) {};

			// set loco direction
			virtual void LocoDirection(__attribute__((unused)) const protocol_t& protocol, __attribute__((unused)) const address_t& address, __attribute__((unused)) const direction_t& direction) {};

			// set loco function
			virtual void LocoFunction(__attribute__((unused)) const protocol_t protocol, __attribute__((unused)) const address_t address, __attribute__((unused)) const function_t function, __attribute__((unused)) const bool on) {};

			// accessory command
			virtual void Accessory(__attribute__((unused)) const protocol_t protocol, __attribute__((unused)) const address_t address, __attribute__((unused)) const accessoryState_t state, __attribute__((unused)) const bool on) {};

		protected:
			Manager* manager;
			const controlID_t controlID;
			const std::string name;
	};

} // namespace

