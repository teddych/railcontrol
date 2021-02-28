/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2021 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include <string>

#include "DataModel/Loco.h"
#include "Hardware/LocoCache.h"

namespace DataModel
{
	class LocoConfig
	{
		public:
			inline LocoConfig()
			:	controlId(ControlNone),
				locoId(LocoNone),
				address(AddressNone),
				protocol(ProtocolNone),
				isInUse(false)
			{
			}

			inline LocoConfig(const DataModel::Loco& loco)
			:	controlId(loco.GetControlID()),
				locoId(loco.GetID()),
				address(loco.GetAddress()),
				protocol(loco.GetProtocol()),
				name(loco.GetName()),
				matchKey(loco.GetMatchKey()),
				isInUse(loco.IsInUse())
			{
			}

			inline LocoConfig& operator=(const DataModel::Loco& loco)
			{
				controlId = loco.GetControlID();
				locoId = loco.GetID();
				address = loco.GetAddress();
				protocol = loco.GetProtocol();
				name = loco.GetName();
				matchKey = loco.GetMatchKey();
				return *this;
			}

			inline LocoConfig& operator=(const Hardware::LocoCacheEntry& loco)
			{
				controlId = loco.GetControlID();
				locoId = loco.GetLocoID();
				address = loco.GetAddress();
				protocol = loco.GetProtocol();
				name = loco.GetName();
				matchKey = loco.GetMatchKey();
				return *this;
			}

			inline ControlID GetControlId()
			{
				return controlId;
			}

			inline LocoID GetLocoId()
			{
				return locoId;
			}

			inline Address GetAddress()
			{
				return address;
			}

			inline Protocol GetProtocol()
			{
				return protocol;
			}

			inline std::string GetName()
			{
				return name;
			}

			inline std::string GetMatchKey()
			{
				return matchKey;
			}

			inline bool IsInUse()
			{
				return isInUse;
			}

		private:
			ControlID controlId;
			LocoID locoId;
			Address address;
			Protocol protocol;
			std::string name;
			std::string matchKey;
			bool isInUse;
	};
} // namespace DataModel
