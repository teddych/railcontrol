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

namespace Hardware
{
	class OpenDccCacheEntry
	{
		public:
			OpenDccCacheEntry()
			{
				speed = 0;
				directionF0 = 0;
				functions = 0;
			}

			unsigned char speed;
			unsigned char directionF0;
			union
			{
					uint32_t functions;
					unsigned char function[4];
			};
	};

	class OpenDccCache
	{
		public:
			OpenDccCache() {};
			~OpenDccCache() {};

			void CreateDataIfNotExists(const address_t address)
			{
				if (cache.count(address) == 1)
				{
					return;
				}
				OpenDccCacheEntry entry;
				cache[address] = entry;
			}

			void SetSpeed(const address_t address, const locoSpeed_t speed)
			{
				CreateDataIfNotExists(address);
				OpenDccCacheEntry entry = cache[address];

				if (speed == 0)
				{
					entry.speed = 0;
				}
				else if (speed > 1000)
				{
					entry.speed = 127;
				}
				else
				{
					entry.speed = (speed >> 3) + 2;
				}

				cache[address] = entry;
			}

			void SetDirection(const address_t address, const direction_t direction)
			{
				CreateDataIfNotExists(address);

				OpenDccCacheEntry entry = cache[address];

				entry.directionF0 &= ~(1 << 5);
				entry.directionF0 |= static_cast<unsigned char>(direction) << 5;

				cache[address] = entry;
			}

			void SetFunction(const address_t address, const function_t function, const bool on)
			{
				CreateDataIfNotExists(address);

				OpenDccCacheEntry entry = cache[address];

				if (function == 0)
				{
					entry.directionF0 &= ~(1 << 4);
					entry.directionF0 |= static_cast<unsigned char>(on) << 4;
				}
				else
				{
					unsigned char shift = function - 1;
					entry.functions &= ~(1 << shift);
					entry.functions |= static_cast<uint32_t>(on) << shift;
				}

				cache[address] = entry;
			}

			OpenDccCacheEntry GetData(const address_t address) const
			{
				if (cache.count(address) == 0)
				{
					OpenDccCacheEntry entry;
					return entry;
				}

				return cache.at(address);
			}

		private:
			std::map<address_t, OpenDccCacheEntry> cache;
	};
} // namespace

