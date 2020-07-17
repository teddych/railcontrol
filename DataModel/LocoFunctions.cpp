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
	std::string LocoFunctions::Serialize() const
	{
		std::string out;
		for (LocoFunctionNr nr = 0; nr < MaxCount; ++nr)
		{
			if (types[nr] == LocoFunctionTypeNone)
			{
				continue;
			}
			out += "f" + std::to_string(static_cast<unsigned int>(nr));
			out += ":" + std::to_string(static_cast<unsigned int>(states[nr]));
			out += ":" + std::to_string(static_cast<unsigned int>(types[nr]));
			out += ":" + std::to_string(static_cast<unsigned int>(icons[nr]));
			if (types[nr] != LocoFunctionTypeTimer)
			{
				continue;
			}
			out += ":" + std::to_string(static_cast<unsigned int>(icons[nr]));
		}
		return out;
	}

	bool LocoFunctions::Deserialize(const std::string& serialized)
	{
		count = serialized.size();
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
		count = 1;
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
			if (nr >= count)
			{
				count = nr + 1;
			}
			functionTexts.pop_front();
			states[nr] = static_cast<LocoFunctionState>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionStateOff));
			functionTexts.pop_front();
			types[nr] = static_cast<LocoFunctionType>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionTypePermanent));
			functionTexts.pop_front();
			icons[nr] = static_cast<LocoFunctionIcon>(Utils::Utils::StringToInteger(functionTexts.front(), LocoFunctionIconDefault));
			functionTexts.pop_front();
			if (nrOfTexts == 4)
			{
				timers[nr] = 0;
				continue;
			}
			timers[nr] = static_cast<LocoFunctionTimer>(Utils::Utils::StringToInteger(functionTexts.front(), 0));
			functionTexts.pop_front();
		}
		return true;
	}

	// FIXME: remove later
	bool LocoFunctions::DeserializeOld(const std::string& serialized)
	{
		if (count > MaxCount)
		{
			count = MaxCount;
		}
		for (LocoFunctionNr i = 0; i < MaxCount; ++i)
		{
			if (i >= count)
			{
				states[i] = LocoFunctionStateOff;
				types[i] = LocoFunctionTypeNone;
				icons[i] = LocoFunctionIconNone;
				timers[i] = 0;
				continue;
			}
			switch (serialized[i])
			{
				case '1':
					states[i] = LocoFunctionStateOn;
					break;

				case '0':
					default:
					states[i] = LocoFunctionStateOff;
					break;
			}
			types[i] = LocoFunctionTypePermanent;
			icons[i] = LocoFunctionIconDefault;
			timers[i] = 0;
		}
		return true;
	}
} // namespace DataModel
