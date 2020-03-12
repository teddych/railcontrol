/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2020 Dominik (Teddy) Mahrer - www.railcontrol.org

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

#include "DataTypes.h"

namespace Hardware
{
	class Z21CacheEntry
	{
		public:
			Z21CacheEntry()
			{
				speed = MinSpeed;
				direction = DirectionRight;
			}

			Z21CacheEntry(const locoSpeed_t speed, const direction_t direction)
			{
				this->speed = speed;
				this->direction = direction;
			}

			locoSpeed_t speed;
			direction_t direction;
	};

	class Z21Cache
	{
		public:
			Z21Cache() {};
			~Z21Cache() {};

			Z21CacheEntry GetData(const address_t address)
			{
				if (cache.count(address) == 1)
				{
					return cache[address];
				}
				Z21CacheEntry entry;
				return entry;
			}

			void SetSpeed(const address_t address, const locoSpeed_t speed)
			{
				Z21CacheEntry entry = GetData(address);
				entry.speed = speed;
				cache[address] = entry;
			}

			locoSpeed_t GetSpeed(const address_t address)
			{
				if (cache.count(address) == 0)
				{
					return MinSpeed;
				}
				return cache[address].speed;
			}

			void SetDirection(const address_t address, const direction_t direction)
			{
				Z21CacheEntry entry = GetData(address);
				entry.direction = direction;
				cache[address] = entry;
			}

			direction_t GetDirection(const address_t address)
			{
				if (cache.count(address) == 0)
				{
					return DirectionRight;
				}
				return cache[address].direction;
			}

			void SetSpeedDirection(const address_t address, const locoSpeed_t speed, const direction_t direction)
			{
				Z21CacheEntry entry(speed, direction);
				cache[address] = entry;
			}

		private:
			std::map<address_t, Z21CacheEntry> cache;
	};
} // namespace

