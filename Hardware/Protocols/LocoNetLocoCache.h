/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2022 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include "DataModel/LocoFunctions.h"
#include "DataTypes.h"

namespace Hardware
{
	namespace Protocols
	{
		class LocoNetLocoCacheEntry
		{
			public:
				inline LocoNetLocoCacheEntry()
				:	address(0),
					orientation(OrientationRight)
				{
				}

				inline void SetAddress(const Address address)
				{
					this->address = address;
				}

				inline Address GetAddress() const
				{
					return address;
				}

				inline void SetSpeed(const Speed speed)
				{
					this->speed = speed;
				}

				inline Speed GetSpeed()
				{
					return speed;
				}

				inline void SetOrientation(const Orientation orientation)
				{
					this->orientation = orientation;
				}

				inline Orientation GetOrientation()
				{
					return orientation;
				}

				inline void SetFunctionState(const DataModel::LocoFunctionNr nr, const DataModel::LocoFunctionState state)
				{
					functions.SetFunctionState(nr, state);
				}

				inline DataModel::LocoFunctionState GetFunctionState(const DataModel::LocoFunctionNr nr) const
				{
					return GetFunctionState(nr);
				}

				Address address;
				Speed speed;
				Orientation orientation;
				DataModel::LocoFunctions functions;
		};

		class LocoNetLocoCache
		{
			public:
				LocoNetLocoCache(const LocoNetLocoCache&) = delete;
				LocoNetLocoCache& operator=(const LocoNetLocoCache&) = delete;

				LocoNetLocoCache()
				{
				}

				virtual ~LocoNetLocoCache()
				{
				}

				inline void SetAddress(const unsigned char slot, const Address address)
				{
					if (slot == 0 || slot > MaxLocoNetSlot)
					{
						return;
					}
					entries[slot].SetAddress(address);
				}

				inline unsigned char GetSlotOfAddress(const Address address)
				{
					for (unsigned char slot = MinLocoNetSlot; slot <= MaxLocoNetSlot; ++slot)
					{
						if (address == entries[slot].GetAddress())
						{
							return slot;
						}
					}
					return 0;
				}

				inline Address GetAddressOfSlot(unsigned char slot)
				{
					if (slot == 0 || slot > MaxLocoNetSlot)
					{
						return 0;
					}
					return entries[slot].GetAddress();
				}

				inline void SetSpeed(const unsigned char slot, const Speed speed)
				{
					if (slot == 0 || slot > MaxLocoNetSlot)
					{
						return;
					}
					entries[slot].SetSpeed(speed);
				}

				inline Speed GetSpeed(const unsigned char slot)
				{
					if (slot == 0 || slot > MaxLocoNetSlot)
					{
						return 0;
					}
					return entries[slot].GetSpeed();
				}

				static const unsigned char MinLocoNetSlot = 1;
				static const unsigned char MaxLocoNetSlot = 119;

			private:
				LocoNetLocoCacheEntry entries[MaxLocoNetSlot + 1];

		};
	} // namespace
} // namespace

