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

#include <deque>
#include <string>

#include "DataModel/LocoFunctions.h"
#include "Utils/Utils.h"

namespace DataModel
{
	LocoFunctions::LocoFunctions()
	{
		for (LocoFunctionNr nr = 0; nr < MaxCount; ++nr)
		{
			entries[nr].nr = nr;
			entries[nr].type = LocoFunctionTypeNone;
			entries[nr].icon = LocoFunctionIconNone;
			entries[nr].timer = 0;
		}
	}

	std::string LocoFunctions::Serialize() const
	{
		std::string out;
		for (LocoFunctionNr nr = 0; nr < MaxCount; ++nr)
		{
			if (entries[nr].type == LocoFunctionTypeNone)
			{
				continue;
			}
			out += "f" + std::to_string(static_cast<unsigned int>(nr));
			out += ":" + std::to_string(static_cast<unsigned int>(entries[nr].state));
			out += ":" + std::to_string(static_cast<unsigned int>(entries[nr].type));
			out += ":" + std::to_string(static_cast<unsigned int>(entries[nr].icon));
			if (entries[nr].type != LocoFunctionTypeTimer)
			{
				continue;
			}
			out += ":" + std::to_string(static_cast<unsigned int>(entries[nr].icon));
		}
		return out;
	}

	bool LocoFunctions::Deserialize(const std::string& serialized)
	{
		size_t count = serialized.size();
		if (count == 0 || serialized[0] == 'f')
		{
			DeserializeNew(serialized);
			return true;
		}
		DeserializeOld(serialized);
		return true;
	}

	bool LocoFunctions::DeserializeNew(__attribute__((unused))  const std::string& serialized)
	{
		std::deque<std::string> functionsSerialized;
		Utils::Utils::SplitString(serialized, "f", functionsSerialized);
		for (std::string& functionSerialized : functionsSerialized)
		{
			if (functionSerialized.size() == 0)
			{
				continue;
			}
			std::deque<std::string> functionTexts;
			Utils::Utils::SplitString(functionSerialized, ":", functionTexts);
			const size_t nrOfTexts = functionTexts.size();
			if (nrOfTexts != 4 && nrOfTexts != 5)
			{
				continue;
			}
			LocoFunctionNr nr = Utils::Utils::StringToInteger(functionTexts.front());
			functionTexts.pop_front();
			entries[nr].state = static_cast<LocoFunctionState>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionStateOff));
			functionTexts.pop_front();
			entries[nr].type = static_cast<LocoFunctionType>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionTypePermanent));
			functionTexts.pop_front();
			entries[nr].icon = static_cast<LocoFunctionIcon>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionIconDefault));
			functionTexts.pop_front();
			if (nrOfTexts == 4)
			{
				entries[nr].timer = 0;
				continue;
			}
			entries[nr].timer = static_cast<LocoFunctionTimer>(Utils::Utils::StringToInteger(functionTexts.front(), 0));
			functionTexts.pop_front();
		}
		return true;
	}

	// FIXME: remove later
	bool LocoFunctions::DeserializeOld(const std::string& serialized)
	{
		size_t count = serialized.size();
		if (count > MaxCount)
		{
			count = MaxCount;
		}
		for (LocoFunctionNr nr = 0; nr < MaxCount; ++nr)
		{
			if (nr >= count)
			{
				entries[nr].state = LocoFunctionStateOff;
				entries[nr].type = LocoFunctionTypeNone;
				entries[nr].icon = LocoFunctionIconNone;
				entries[nr].timer = 0;
				continue;
			}
			switch (serialized[nr])
			{
				case '1':
					entries[nr].state = LocoFunctionStateOn;
					break;

				case '0':
					default:
						entries[nr].state = LocoFunctionStateOff;
					break;
			}
			entries[nr].type = LocoFunctionTypePermanent;
			entries[nr].icon = LocoFunctionIconDefault;
			entries[nr].timer = 0;
		}
		return true;
	}
} // namespace DataModel
