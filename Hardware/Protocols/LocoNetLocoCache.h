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

namespace Hardware
{
	namespace Protocols
	{
		class LocoNetLocoCacheEntry
		{
			public:
				inline LocoNetLocoCacheEntry()
				:	address(0)
				{
				}

				explicit inline operator Address() const
				{
					return address;
				}

				inline LocoNetLocoCacheEntry& operator=(const Address address)
				{
					this->address = address;
					return *this;
				}

				Address address;
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

				inline void Set(const unsigned char slot, const Address address)
				{
					entries[slot] = address;
				}

				static const unsigned char MinLocoNetSlot = 1;
				static const unsigned char MaxLocoNetSlot = 119;

			private:
				LocoNetLocoCacheEntry entries[MaxLocoNetSlot + 1];

		};
	} // namespace
} // namespace

