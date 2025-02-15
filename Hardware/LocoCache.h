/*
RailControl - Model Railway Control Software

Copyright (c) 2017-2025 by Teddy / Dominik Mahrer - www.railcontrol.org

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

#include <cstring>
#include <string>
#include <map>
#include <vector>

#include "DataTypes.h"
#include "DataModel/LocoFunctions.h"

class Manager;

namespace Hardware
{
	class LocoCacheEntrySlave
	{
		public:
			LocoCacheEntrySlave() = delete;

			inline LocoCacheEntrySlave(const Protocol protocol,
				const Address address,
				const std::string matchKey)
			:	protocol(protocol),
				address(address),
				matchKey(matchKey),
				locoID(LocoNone)
			{
			}

			inline Protocol GetProtocol() const
			{
				return protocol;
			}

			inline Address GetAddress() const
			{
				return address;
			}

			inline const std::string& GetMatchKey() const
			{
				return matchKey;
			}

			inline void SetLocoID(LocoID locoID)
			{
				this->locoID = locoID;
			}

			inline LocoID GetLocoID() const
			{
				return locoID;
			}

		private:
			Protocol protocol;
			Address address;
			std::string matchKey;
			LocoID locoID;
	};

	class LocoCacheEntry
	{
		public:
			LocoCacheEntry() = delete;

			inline LocoCacheEntry(const ControlID controlId)
			:	controlId(controlId),
				locoId(LocoNone),
				protocol(ProtocolNone),
				address(AddressNone),
				type(LocoTypeLoco),
				matchKey("")
			{
			}

			inline ControlID GetControlID() const
			{
				return controlId;
			}

			inline LocoID GetLocoID() const
			{
				return locoId;
			}

			inline void SetLocoID(const LocoID locoId)
			{
				this->locoId = locoId;
			}

			inline const std::string& GetName() const
			{
				return name;
			}

			inline void SetName(const std::string& name)
			{
				this->name = name;
			}

			inline Protocol GetProtocol() const
			{
				return protocol;
			}

			inline void SetProtocol(const Protocol protocol)
			{
				this->protocol = protocol;
			}

			inline Address GetAddress() const
			{
				return address;
			}

			inline void SetAddress(const Address address)
			{
				this->address = address;
			}

			inline LocoType GetType() const
			{
				return type;
			}

			inline void SetType(LocoType type)
			{
				this->type = type;
			}

			inline void SetFunction(const DataModel::LocoFunctionNr nr,
				const DataModel::LocoFunctionType type,
				const DataModel::LocoFunctionIcon icon,
				const DataModel::LocoFunctionTimer timer)
			{
				functions.SetFunction(nr, type, icon, timer);
			}

			inline void ClearFunction(const DataModel::LocoFunctionNr nr)
			{
				functions.ClearFunction(nr);
			}

			inline std::vector<DataModel::LocoFunctionEntry> GetFunctionStates() const
			{
				return functions.GetFunctionStates();
			}

			inline const std::string& GetMatchKey() const
			{
				return matchKey;
			}

			inline void SetMatchKey(const std::string& matchKey)
			{
				this->matchKey = matchKey;
			}

			inline void SetMatchKey(const unsigned int matchKey)
			{
				this->matchKey = std::to_string(matchKey);
			}

			inline void AddSlave(const Protocol protocol, const Address address, const std::string& matchKey)
			{
				slaves.push_back(LocoCacheEntrySlave(protocol, address, matchKey));
			}

			void UpdateSlaves(Manager* manager);

			inline std::vector<LocoID> GetSlaveIDs() const
			{
				std::vector<LocoID> out;
				for (auto const & slave : slaves)
				{
					const LocoID slaveID = slave.GetLocoID();
					if (slaveID == LocoNone)
					{
						continue;
					}
					out.push_back(slaveID);
				}
				return out;
			}

		private:
			const ControlID controlId;
			LocoID locoId;
			std::string name;
			Protocol protocol;
			Address address;
			LocoType type;
			std::string matchKey;
			DataModel::LocoFunctions functions;
			std::vector<LocoCacheEntrySlave> slaves;
	};

	class LocoCache
	{
		public:
			inline LocoCache(const ControlID controlId,
				Manager* const manager)
			:	controlId(controlId),
				manager(manager)
			{
			}

			LocoCache() = delete;

			LocoCache(const LocoCache& rhs) = delete;

			LocoCache& operator= (const LocoCache& rhs) = delete;

			inline ControlID GetControlId() const
			{
				return controlId;
			}

			inline void Save(LocoCacheEntry& entry)
			{
				const std::string& oldMatchKey = entry.GetMatchKey();
				Save(entry, oldMatchKey);
			}

			void Save(LocoCacheEntry& entry, const std::string& oldMatchKey);

			LocoID Delete(const std::string& matchKey);

			inline const LocoCacheEntry Get(const std::string& matchKey) const
			{
				return entries.count(matchKey) == 0 ? LocoCacheEntry(controlId) : entries.at(matchKey);
			}

			inline const std::map<std::string,LocoCacheEntry>& GetAll() const
			{
				return entries;
			}

			void SetLocoId(const LocoID locoId, const std::string& matckKey);

			inline void UpdateSlaves()
			{
				for (auto & entry : entries)
				{
					entry.second.UpdateSlaves(manager);
				}
			}

		private:
			const ControlID controlId;
			Manager* const manager;
			std::map<std::string,LocoCacheEntry> entries;
	};
} // namespace Hardware

